#include "glGribShell.h"
#include "glGribWindowSet.h"
#include "glGribWindow.h"

#include <iostream>
#include <exception>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>


void glGrib::Shell::do_close (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  gwindow->shouldClose ();
}

void glGrib::Shell::do_snapshot (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
   gwindow->framebuffer ();
}

void glGrib::Shell::do_sleep (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  unlock ();
  sleep (std::stoi (args[1]));
  lock ();
}

void glGrib::Shell::do_clone (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  gwindow->setCloned ();
}

void glGrib::Shell::do_set (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  int argc = args.size ();
  const char * argv[argc];
  
  for (int i = 0; i < args.size (); i++)
    argv[i] = args[i].c_str ();
  
  glGrib::Options opts = gwindow->scene.getOptions ();
  opts.window = gwindow->getOptions ();
  opts.shell = this->opts;
  
  glGrib::OptionsParser p;
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
          for (hof_t::const_iterator it = begin (); it != end (); it++) 
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
  
              for (opts_set_it it1 = opts_set.begin (); it1 != opts_set.end (); it1++)
              for (opts_set_it it2 = opts_set.begin (); it2 != opts_set.end (); it2++)
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
  
          for (opts_set_it it = opts_set.begin (); it != opts_set.end (); it++)
            {
              hof_t::iterator it_hof = find (*it);
              const sof_t & sof = it_hof->second;
              sof ();
            }
          
  
          if (opts_not.size () > 0)
            {
              std::cout << "Warning: the following options where not applied : " << std::endl;
              for (opts_set_it it = opts_not.begin (); it != opts_not.end (); it++)
                std::cout << "   " << *it << std::endl;
            }
  
        }
  
        std::set<std::string> opts_set;
        std::set<std::string> opts_not;
      };
  
      hof_t hof;
  
      hof.add ("--window.height"             , [&opts,gwindow]() { gwindow->setOptions                     (opts.window             );  });
      hof.add ("--window.width"              , [&opts,gwindow]() { gwindow->setOptions                     (opts.window             );  });
      hof.add ("--window.title"              , [&opts,gwindow]() { gwindow->setOptions                     (opts.window             );  });
      hof.add ("--window.position"           , [&opts,gwindow]() { gwindow->setOptions                     (opts.window             );  });
      hof.add ("--view"                      , [&opts,gwindow]() { gwindow->scene.setViewOptions           (opts.view               );  });
      hof.add ("--landscape"                 , [&opts,gwindow]() { gwindow->scene.setLandscapeOptions      (opts.landscape          );  });
      hof.add ("--grid"                      , [&opts,gwindow]() { gwindow->scene.setGridOptions           (opts.grid               );  });
      hof.add ("--land"                      , [&opts,gwindow]() { gwindow->scene.setLandOptions           (opts.land               );  });
      hof.add ("--ticks"                     , [&opts,gwindow]() { gwindow->scene.setTicksOptions          (opts.ticks              );  });
      hof.add ("--grid.color"                , [&opts,gwindow]() { gwindow->scene.setGridColorOptions      (opts.grid.color         );  });
      hof.add ("--grid.scale"                , [&opts,gwindow]() { gwindow->scene.setGridScaleOptions      (opts.grid.scale         );  });
      hof.add ("--coast"                     , [&opts,gwindow]() { gwindow->scene.setCoastOptions          (opts.coast              );  });
      hof.add ("--cities"                    , [&opts,gwindow]() { gwindow->scene.setCitiesOptions         (opts.cities             );  });
      hof.add ("--border"                    , [&opts,gwindow]() { gwindow->scene.setBorderOptions         (opts.border             );  });
      hof.add ("--rivers"                    , [&opts,gwindow]() { gwindow->scene.setRiversOptions         (opts.rivers             );  });
      hof.add ("--colorbar"                  , [&opts,gwindow]() { gwindow->scene.setColorBarOptions       (opts.colorbar           );  });
      hof.add ("--mapscale"                  , [&opts,gwindow]() { gwindow->scene.setMapScaleOptions       (opts.mapscale           );  });
      hof.add ("--scene.image"               , [&opts,gwindow]() { gwindow->scene.setImageOptions          (opts.scene.image        );  });
      hof.add ("--scene.text"                , [&opts,gwindow]() { gwindow->scene.setTextOptions           (opts.scene.text         );  });
      hof.add ("--scene.date"                , [&opts,gwindow]() { gwindow->scene.setDateOptions           (opts.scene.date         );  });
      hof.add ("--scene.title"               , [&opts,gwindow]() { gwindow->scene.setTitleOptions          (opts.scene.title        );  });
      hof.add ("--scene.light"               , [&opts,gwindow]() { gwindow->scene.setLightOptions          (opts.scene.light        );  });
      hof.add ("--scene.lon_at_hour"         , [&opts,gwindow]() { gwindow->scene.setSceneOptions          (opts.scene              );  });
      hof.add ("--scene.rotate_earth"        , [&opts,gwindow]() { gwindow->scene.setSceneOptions          (opts.scene              );  });
      hof.add ("--shell.prompt.on"           , [&opts,this   ]() { this->opts.prompt.on =                   opts.shell.prompt.on;       });

#define SFO(j) \
do { \
          hof.add ("--field[" #j "]"             , [&opts,gwindow]() { gwindow->scene.setFieldOptions          (j, opts.field[j]        );  });  \
          hof.add ("--field[" #j "].palette"     , [&opts,gwindow]() { gwindow->scene.setFieldPaletteOptions   (j, opts.field[j].palette);  });  \
} while (0)
      SFO  (0); SFO  (1); SFO  (2); SFO  (3);
      SFO  (4); SFO  (5); SFO  (6); SFO  (7);
      SFO  (8); SFO  (9); SFO (10); SFO (11);
#undef SFO
          
      std::set<std::string> seen = p.getSeenOptions ();
    
      for (std::set<std::string>::iterator it = seen.begin (); it != seen.end (); it++)
        hof.set (*it);
    
      hof.run ();
    
    }
  
}

void glGrib::Shell::do_window_select (const std::vector<std::string> & args, glGrib::Window * gwindow)
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

std::vector<int>
glGrib::Shell::do_window (const std::vector<std::string> & args, glGrib::Window * gwindow)
{

  if (args.size () == 1)
    return do_window_list (args, gwindow);
  else if (args.size () == 2)
    do_window_select (args, gwindow);

  return std::vector<int> ();
}

std::string
glGrib::Shell::do_help (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  std::string help;

  glGrib::Options opts = gwindow->scene.getOptions ();
  glGrib::OptionsParser p;
  opts.traverse ("", &p);
 
  for (int i = 1; i < args.size (); i++)
    help += p.getHelp (args[i], true);      

  return help;
}


std::vector<std::string>
glGrib::Shell::do_get (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  std::vector<std::string> list;

  glGrib::Options opts = gwindow->scene.getOptions ();
  glGrib::OptionsParser p;
  opts.traverse ("", &p);

  for (int i = 1; i < args.size (); i++)
    p.getValue (&list, args[i], true);

  return list;
}

std::vector<int>
glGrib::Shell::do_window_list (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  std::vector<int> list;
  for (const auto w : *wset)
    list.push_back (w->id ());
  return list;
}

void glGrib::Shell::execute (const std::vector<std::string> & args)
{
  
  glGrib::Window * gwindow = wset->getWindowById (windowid);

  gwindow->makeCurrent ();

#define glGribShellIfCommand(command) \
  do                                                       \
  {                                                        \
    if (#command == args[0]) process_##command (args, gwindow); \
  } while (0)


  glGribShellIfCommand (close);
  glGribShellIfCommand (snapshot);
  glGribShellIfCommand (sleep);
  glGribShellIfCommand (clone);
  glGribShellIfCommand (get);
  glGribShellIfCommand (set);
  glGribShellIfCommand (window);
  glGribShellIfCommand (help);

#undef glGribShellIfCommand
  
}

