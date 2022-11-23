#include "glGribShell.h"
#include "glGribWindowSet.h"
#include "glGribRender.h"
#include "glGribContainer.h"
#include "glGribResolve.h"
#include "glGribTrigonometry.h"

#include <iostream>
#include <exception>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>

#include <cstring>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

namespace glGrib
{

void Shell::do_close (const std::vector<std::string> & args, Render * gwindow)
{
  gwindow->shouldClose ();
}

void Shell::do_snapshot (const std::vector<std::string> & args, Render * gwindow)
{
  gwindow->framebuffer ();
}

void Shell::do_sleep (const std::vector<std::string> & args, Render * gwindow)
{
  unlock ();
  sleep (std::stoi (args[1]));
  lock ();
}

void Shell::do_clone (const std::vector<std::string> & args, Render * gwindow)
{
  gwindow->setCloned ();
}

void Shell::do_clear (const std::vector<std::string> & args, Render * gwindow)
{
  gwindow->setCleared ();
}

void Shell::do_list (const std::vector<std::string> & args, Render * gwindow)
{
  if (args.size () < 2)
    return;

  try
    {
      auto cont = Container::create (args[1]);
      for (auto it = cont->begin (); it != cont->end (); ++it)
        listStr.push_back (*it);
      delete cont;
    } 
  catch (...)
    {
    }
}

void Shell::do_resolve (const std::vector<std::string> & args, Render * gwindow)
{
  if (args.size () < 2)
    return;

  listStr.push_back (Resolve (args[1]));
}

void Shell::do_goto (const std::vector<std::string> & args, Render * gwindow)
{
  if (args.size () < 2)
    return;

  auto & scene = gwindow->getScene ();

  auto field = scene.getCurrentField ();

  if (field == nullptr)
    return;

  int gridpoint = -1;

  if ((args[1] == "min") || (args[1] == "max"))
    {
      auto & values = field->getValues ();
      if (values.size () != 1)
        return;
      auto v = values[0];
  
      float * b = &v[0], * e = &v[0] + v->size ();

      gridpoint = ((args[1] == "min") ?  std::min_element (b, e) : std::max_element (b, e)) - b;
    }
  else
    {
      gridpoint = std::stoi (args[1]);
    }

  if (gridpoint < 0)
    return;

  const_GeometryPtr geometry = field->getGeometry ();

  auto & view = scene.getView ();

  OptionsView view_opts = view.getOptions ();
  float lon, lat;
  geometry->index2latlon (gridpoint, &lat, &lon);
  view_opts.lat = lat * rad2deg;
  view_opts.lon = lon * rad2deg;
  view.setOptions (view_opts);
}

void Shell::do_set (const std::vector<std::string> & args, Render * gwindow)
{
  int argc = args.size ();
  const char * argv[argc];
  
  for (size_t i = 0; i < args.size (); i++)
    argv[i] = args[i].c_str ();
  
  auto & scene = gwindow->getScene ();

  Options opts = scene.getOptions ();

  opts.render = gwindow->getOptions ();
  opts.shell = this->opts;
  
  OptionsParser p;
  opts.traverse ("", &p);
  
  if (p.parse (argc, argv))
    {
      typedef std::function<void ()> sof_t;
  
      class hof_t : public std::map<std::string,sof_t>
      {
      public:
  
        typedef std::set<std::string>::iterator opts_set_it;
  
        void add (const std::string & name, sof_t func)
        {
          insert (std::pair<std::string,sof_t>(name, func));
        }
        sof_t get (const std::string & name)
        {
          hof_t::iterator it = find (name);
          return it->second;
        }
        void set (const std::string & opt_name)
        {
          // Choose longest (most specialized) option available
          std::string specialized;
          for (hof_t::const_iterator it = begin (); it != end (); ++it) 
            {
              const std::string & name = it->first;
              if ((name == opt_name.substr (0, name.size ())) && (name.size () > specialized.size ()))
                specialized = name;
            }
          if (specialized != "")
            opts_set.insert (specialized);
          else
            opts_not.insert (opt_name);
        }
        void run ()
        {
  
          // Remove options which already exist in a less specialized (shorter) form
          while (1)
            {
              int tt;
  
            again:
  
              tt = 0;
  
              for (opts_set_it it1 = opts_set.begin (); it1 != opts_set.end (); ++it1)
              for (opts_set_it it2 = opts_set.begin (); it2 != opts_set.end (); ++it2)
                {
                  const std::string & n1 = *it1, & n2 = *it2;
                  if (n1 == n2)
                    continue;
                  if (n1.substr (0, n2.size ()) == n2) // n2 shorter than n1: remove n1
                    {
                      opts_set.erase (n1);
                      tt++;
                    }
                  if (n2.substr (0, n1.size ()) == n1) // n1 shorter than n2: remove n2
                    {
                      opts_set.erase (n2);
                      tt++;
                    }
                  if (tt > 0)
                    goto again;
                }
              if (tt == 0)
                break;
            }
  
          for (opts_set_it it = opts_set.begin (); it != opts_set.end (); ++it)
            {
              hof_t::iterator it_hof = find (*it);
              const sof_t & sof = it_hof->second;
              sof ();
            }
          
  
          if (opts_not.size () > 0)
            {
              std::cout << "Warning: the following options where not applied : " << std::endl;
              for (opts_set_it it = opts_not.begin (); it != opts_not.end (); ++it)
                std::cout << "   " << *it << std::endl;
            }
  
        }
  
        std::set<std::string> opts_set;
        std::set<std::string> opts_not;
      };
  
      hof_t hof;
  
      hof.add ("--render.height"             , [&opts,&scene,gwindow]() { gwindow->setOptions            (opts.render             );  });
      hof.add ("--render.width"              , [&opts,&scene,gwindow]() { gwindow->setOptions            (opts.render             );  });
      hof.add ("--render.title"              , [&opts,&scene,gwindow]() { gwindow->setOptions            (opts.render             );  });
      hof.add ("--render.position"           , [&opts,&scene,gwindow]() { gwindow->setOptions            (opts.render             );  });
      hof.add ("--view"                      , [&opts,&scene,gwindow]() { scene.setViewOptions           (opts.view               );  });
      hof.add ("--landscape"                 , [&opts,&scene,gwindow]() { scene.setLandscapeOptions      (opts.landscape          );  });
      hof.add ("--grid"                      , [&opts,&scene,gwindow]() { scene.setGridOptions           (opts.grid               );  });
      hof.add ("--land"                      , [&opts,&scene,gwindow]() { scene.setLandOptions           (opts.land               );  });
      hof.add ("--ticks"                     , [&opts,&scene,gwindow]() { scene.setTicksOptions          (opts.ticks              );  });
      hof.add ("--grid.color"                , [&opts,&scene,gwindow]() { scene.setGridColorOptions      (opts.grid.color         );  });
      hof.add ("--grid.scale"                , [&opts,&scene,gwindow]() { scene.setGridScaleOptions      (opts.grid.scale         );  });
      hof.add ("--coast"                     , [&opts,&scene,gwindow]() { scene.setCoastOptions          (opts.coast              );  });
      hof.add ("--cities"                    , [&opts,&scene,gwindow]() { scene.setCitiesOptions         (opts.cities             );  });
      hof.add ("--border"                    , [&opts,&scene,gwindow]() { scene.setBorderOptions         (opts.border             );  });
      hof.add ("--land"                      , [&opts,&scene,gwindow]() { scene.setLandOptions           (opts.land               );  });
      hof.add ("--rivers"                    , [&opts,&scene,gwindow]() { scene.setRiversOptions         (opts.rivers             );  });
      hof.add ("--colorbar"                  , [&opts,&scene,gwindow]() { scene.setColorBarOptions       (opts.colorbar           );  });
      hof.add ("--mapscale"                  , [&opts,&scene,gwindow]() { scene.setMapScaleOptions       (opts.mapscale           );  });
      hof.add ("--scene.image"               , [&opts,&scene,gwindow]() { scene.setImageOptions          (opts.scene.image        );  });
      hof.add ("--scene.text"                , [&opts,&scene,gwindow]() { scene.setTextOptions           (opts.scene.text         );  });
      hof.add ("--scene.date"                , [&opts,&scene,gwindow]() { scene.setDateOptions           (opts.scene.date         );  });
      hof.add ("--scene.title"               , [&opts,&scene,gwindow]() { scene.setTitleOptions          (opts.scene.title        );  });
      hof.add ("--scene.light"               , [&opts,&scene,gwindow]() { scene.setLightOptions          (opts.scene.light        );  });
      hof.add ("--scene.lon_at_hour"         , [&opts,&scene,gwindow]() { scene.setSceneOptions          (opts.scene              );  });
      hof.add ("--scene.rotate_earth"        , [&opts,&scene,gwindow]() { scene.setSceneOptions          (opts.scene              );  });
      hof.add ("--departements"              , [&opts,&scene,gwindow]() { scene.setDepartementsOptions   (opts.departements       );  });
      hof.add ("--shell.prompt.on"           , [&opts,&scene,this   ]() { this->opts.prompt.on =          opts.shell.prompt.on;       });

#define SFO(j) \
do { \
          hof.add ("--field[" #j "]"             , [&opts,&scene,gwindow]() { scene.setFieldOptions          (j, opts.field[j]        );  });  \
          hof.add ("--field[" #j "].palette"     , [&opts,&scene,gwindow]() { scene.setFieldPaletteOptions   (j, opts.field[j].palette);  });  \
} while (0)
      SFO  (0); SFO  (1); SFO  (2); SFO  (3);
      SFO  (4); SFO  (5); SFO  (6); SFO  (7);
      SFO  (8); SFO  (9); SFO (10); SFO (11);
#undef SFO
          
      std::set<std::string> seen = p.getSeenOptions ();
    
      for (std::set<std::string>::iterator it = seen.begin (); it != seen.end (); ++it)
        hof.set (*it);
    
      hof.run ();
    
    }
  
}

void Shell::do_window_select (const std::vector<std::string> & args, Render * gwindow)
{
  try
    {
      windowid = std::stoi (args[1]);
    }
  catch (...)
    {
      std::cout << "window command expects an integer value" << std::endl;
    }
}

void Shell::do_window (const std::vector<std::string> & args, Render * gwindow)
{
  if (args.size () == 1)
    do_window_list (args, gwindow);
  else if (args.size () == 2)
    do_window_select (args, gwindow);
}

void Shell::do_help (const std::vector<std::string> & args, Render * gwindow)
{
  std::string help;

  Options opts = gwindow->getScene ().getOptions ();
  OptionsParser p;
  opts.traverse ("", &p);
 
  for (size_t i = 1; i < args.size (); i++)
    help += p.getHelp (args[i], true);      

  listStr.push_back (help);
}


namespace
{
template <typename F>
void getopts 
  (const std::vector<std::string> & args, 
   std::vector<std::string> & listStr,
   Render * gwindow, F f)
{
  Options opts0 = gwindow->getScene ().getOptions ();
  OptionsParser p0;
  opts0.traverse ("", &p0);

  if ((args.size () > 1) && (args[1] == "+diff"))
    {
      std::vector<std::string> a;
      a.push_back (args[0]);
      for (size_t i = 2; i < args.size (); i++)
        a.push_back (args[i]);
      Options opts1;
      OptionsParser p1;
      opts1.traverse ("", &p1);
      for (size_t i = 1; i < a.size (); i++)
        f (listStr, &p0, &p1, a[i]);
    }
  else if ((args.size () > 1) && (args[1] == "+base"))
    {
      Options opts1;
      OptionsParser p1;
      opts1.traverse ("", &p1);
      for (size_t i = 2; i < args.size (); i++)
        f (listStr, &p1, nullptr, args[i]);
    }
  else
    {
      for (size_t i = 1; i < args.size (); i++)
        f (listStr, &p0, nullptr, args[i]);
    }
}
};

void Shell::do_json (const std::vector<std::string> & args, Render * gwindow)
{
  getopts (args, listStr, gwindow, 
  [] (std::vector<std::string> & listStr, 
      OptionsParser * pA, 
      OptionsParser * pB, 
      const std::string a)
  {
    listStr.push_back (pA->getJSON (a, true, true, pB));
  });
}

void Shell::do_get (const std::vector<std::string> & args, Render * gwindow)
{
  getopts (args, listStr, gwindow, 
  [] (std::vector<std::string> & listStr, 
      OptionsParser * pA, 
      OptionsParser * pB, 
      const std::string a)
  {
    pA->getValue (&listStr, a, true, true, pB);
  });
}

void Shell::do_window_list (const std::vector<std::string> & args, Render * gwindow)
{
  for (const auto w : *wset)
    listStr.push_back (std::to_string (w->id ()));
}

void Shell::execute (const std::vector<std::string> & args)
{
  if (args.size () == 0)
    return;

  listStr.clear ();

  Render * gwindow = wset->getWindowById (windowid);
  if (gwindow == nullptr)
    {
      for (auto w : *wset)
        {
          gwindow = w;
          break;
	}
      if (gwindow == nullptr)
        return;
      windowid = gwindow->id ();
    }

  gwindow->makeCurrent ();

#define glGribShellIfCommand(command) \
  do                                       \
  {                                        \
    if (#command == args[0])               \
      {                                    \
        do_##command      (args, gwindow); \
        process_##command (args, gwindow); \
      }                                    \
  } while (0)


  glGribShellIfCommand (close);
  glGribShellIfCommand (snapshot);
  glGribShellIfCommand (sleep);
  glGribShellIfCommand (clone);
  glGribShellIfCommand (clear);
  glGribShellIfCommand (get);
  glGribShellIfCommand (json);
  glGribShellIfCommand (list);
  glGribShellIfCommand (resolve);
  glGribShellIfCommand (set);
  glGribShellIfCommand (window);
  glGribShellIfCommand (help);
  glGribShellIfCommand (goto);

#undef glGribShellIfCommand
  
}

void Shell::setWindowSet (WindowSet * _wset)
{
  if (wset != nullptr)
    throw std::runtime_error (std::string ("wset already set"));
  wset = _wset;
}

WindowSet * Shell::getWindowSet ()
{
  return wset;
}

void Shell::clearWindowSet ()
{
  delete wset;
  wset = nullptr;
}

Render * Shell::getWindow ()
{
  return wset->getWindowById (windowid);
}

Render * Shell::getFirstWindow ()
{
  return wset->getFirstWindow ();
}

}
