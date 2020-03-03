#include "glGribShell.h"
#include "glGribWindowSet.h"

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


namespace glGrib
{
glGrib::Shell Shell0;
}

char * glGrib::Shell::optionGenerator (const char * text, int state)
{

  if (! state) 
    {
      og.list_index = 0;
      og.text_len = strlen (text);
    }

  std::string tt = text;

  for (; og.list_index < getsetoptions.size (); og.list_index++)
    if (getsetoptions[og.list_index].substr (0, og.text_len) == tt)
      return strdup (getsetoptions[og.list_index++].c_str ());  // Note the ++

  return nullptr;
}

char * shellOptionGenerator (const char * text, int state)
{
  return glGrib::Shell0.optionGenerator (text, state);
}

char ** shellCompletion (const char * text, int start, int end)
{
  char * line = rl_line_buffer;
  if ((strncmp (line, "set ", 4) == 0) || (strncmp (line, "get ", 4) == 0))
    return rl_completion_matches (text, shellOptionGenerator);
  return nullptr;
}

glGrib::Shell::Shell ()
{
  glGrib::Options opts;
  glGrib::OptionsParser p;
  opts.traverse ("", &p);
  p.getOptions (&getsetoptions);
  rl_attempted_completion_function = shellCompletion;
}

static void help ()
{
  std::cout << "Unknown command" << std::endl;
}

void glGrib::Shell::execute (const std::string & _line, glGrib::Window * gwindow)
{
  std::string cmd;
  std::vector<std::string> args;
  
  gwindow->makeCurrent ();

  std::string line = _line;

  try 
    {
      cmd = glGrib::OptionsUtil::nextToken (&line);

      if (cmd == "")
        return;
      if (cmd[0] == '#')
        return;
     
      while (1)
        {
          std::string arg = glGrib::OptionsUtil::nextToken (&line);
          if (arg == "") 
            break;
          if (arg[0] == '#')
            break;
          args.push_back (arg);
        }
    }
  catch (const std::runtime_error & e)
    {
      std::cout << "Malformed command " << _line << std::endl;
      std::cout << e.what () << std::endl;
      return;
    }


  if (cmd == "close")
    {
      gwindow->shouldClose ();
    }
  else if (cmd == "snapshot")
    {
      gwindow->framebuffer ();
    }
  else if (cmd == "sleep")
    {
      unlock ();
      sleep (std::stoi (args[0]));
      lock ();
    }
  else if (cmd == "clone")
    {
      gwindow->setCloned ();
    }
  else if (cmd == "show")
    {
      glGrib::Options opts = gwindow->scene.getOptions ();
      glGrib::OptionsParser::print (opts);
    }
  else if (cmd == "get")
    {
      glGrib::Options opts = gwindow->scene.getOptions ();
      glGrib::OptionsParser p;
      opts.traverse ("", &p);
 
      for (int i = 0; i < args.size (); i++)
        p.display (args[i], true);      

    }
  else if (cmd == "set")
    {

      int argc = 1 + args.size ();
      const char * argv[argc];
      argv[0] = "glGrib::";
      
      for (int i = 0; i < args.size (); i++)
        argv[1+i] = args[i].c_str ();
      
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
  else if ((cmd == "window") && (args.size () == 0))
    {
      std::cout << "Window list:" << std::endl;
      for (glGrib::WindowSet::const_iterator it = wset->begin (); it != wset->end (); it++)
        {
          int id = (*it)->id ();
          std::cout << (windowid == id ? " > " : "   ") << id << std::endl;
        }
    }
  else if ((cmd == "window") && (args.size () == 1))
    {
      try
        {
          windowid = std::stoi (args[0]);
        }
      catch (...)
        {
          std::cout << "window command expects an integer value" << std::endl;
        }
    }

}

static 
void * _run (void * data)
{
  glGrib::Shell * shell = (glGrib::Shell *)data;
  shell->run ();
  return nullptr;
}


void glGrib::Shell::start (glGrib::WindowSet * ws)
{
  wset = ws;
  pthread_create(&thread, nullptr, _run, this);
}


void glGrib::Shell::runInt ()
{
  if (read_history (".glGrib::History") != 0)
    write_history (".glGrib::History");
  while (wset->size () > 0)
    {
      char * line = readline ("glGrib::> ");

      if (line == nullptr)
        {
          lock ();
          wset->close ();
          unlock ();
          break;
        }

      if (strlen (line) > 0) 
        add_history (line);
 
      lock ();
      {
        if (wset->size ())
          {
            glGrib::Window * gwindow = wset->getWindowById (windowid);
	    if (gwindow == nullptr)
              gwindow = wset->getFirstWindow ();
            if (gwindow != nullptr)
              execute (line, gwindow);
	  }
      }
      unlock ();
      
      append_history (1, ".glGrib::History");

      free (line);
      
      if (closed ()) 
        break;

    }
}

void glGrib::Shell::runOff ()
{
  std::ifstream fp (opts.script);

  if (fp)
    {
      std::string line; 
      while (std::getline (fp, line)) 
        {
          if (wset->size () == 0)
            break;
          lock ();
          glGrib::Window * gwindow = wset->getWindowById (windowid);
          if (gwindow == nullptr)
            gwindow = wset->getFirstWindow ();
          if (gwindow != nullptr)
            execute (line, gwindow);
          unlock ();
        }
      if (opts.prompt.on)
        {
         runInt ();
        }
      else
        {
          lock ();
          wset->close ();
          unlock ();
        }
    }
  else
    {
      throw std::runtime_error (std::string ("Cannot open :") + opts.script);
    }
}

void glGrib::Shell::run ()
{
  if (opts.script != "")
    runOff ();
  else
    runInt ();
}

void glGrib::Shell::setup (const glGrib::OptionsShell & o)
{ 
  opts = o;
}




