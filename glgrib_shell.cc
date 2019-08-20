#include "glgrib_shell.h"

#include <iostream>
#include <exception>

#include <readline/readline.h>
#include <readline/history.h>


glgrib_shell Shell;

static std::string next_token (std::string * line)
{
  while (line->length () && (*line)[0] == ' ')
    *line = line->substr (1);

  std::string token = std::string ("");

  int q = 0, qq = 0;

  while (line->length ())
    {
      char c = (*line)[0];
      *line = line->substr (1);


      if ((qq == 0) && (q == 0))
        {
          if (c == ' ')
            break;
          if (c == '"')
            {
              qq = 1;
              goto cont;
            }
          if (c == '\'')
            {
              q = 1;
              goto cont;
            }
        }
      else
        {
          if ((c == '"') && (qq == 1))
            {
              qq = 0;
              goto cont;
            }
          if ((c == '\'') && (q == 1))
            {
              q = 0;
              goto cont;
            }
        }

      if (c == '\\')
        {
          if (line->length ())
            {
              c = (*line)[0];
              *line = line->substr (1);
            }
          else
            throw std::runtime_error (std::string ("Stray '\\'"));
        }
      token.push_back (c);

cont:
      continue;
    }

  if (qq || q)
    throw std::runtime_error (std::string ("Unterminated character string"));

  return token;
}

static void help ()
{
  std::cout << "Unknown command" << std::endl;
}

void glgrib_shell::execute (const std::string & _line, glgrib_window * gwindow)
{
  std::string cmd;
  std::vector<std::string> args;
  
  gwindow->makeCurrent ();

  std::string line = _line;

  try 
    {
      cmd = next_token (&line);

      if (cmd == "")
        return;
     
      while (1)
        {
          std::string arg = next_token (&line);
          if (arg == "") 
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


  if ((cmd == "exit") || (cmd == "quit"))
    {
      close = 1;
    }
  else if (cmd == "get")
    {
      glgrib_options opts = gwindow->scene.getOptions ();
      glgrib_options_parser p;
      opts.traverse ("", &p);
 
      for (int i = 0; i < args.size (); i++)
        p.display (args[i]);      

    }
  else if (cmd == "set")
    {

      int argc = 1 + args.size ();
      const char * argv[argc];
      argv[0] = "glgrib";
      
      for (int i = 0; i < args.size (); i++)
        argv[1+i] = args[i].c_str ();
      
      glgrib_options opts = gwindow->scene.getOptions ();
      glgrib_options_parser p;
      opts.traverse ("", &p);
      
      if (p.parse (argc, argv))
        {
          if (p.seenOption ("--view"))
            gwindow->scene.setViewOpts (opts.view);
          if (p.seenOption ("--landscape"))
            gwindow->scene.setLandscapeOpts (opts.landscape);
          if (p.seenOption ("--grid"))
            gwindow->scene.setGridOpts (opts.grid);
          if (p.seenOption ("--coastlines"))
            gwindow->scene.setCoastlinesOpts (opts.coastlines);

#define SFO(j) \
do {                                                           \
          if (p.seenOption ("--field[" #j "]"))                \
            gwindow->scene.setFieldOpts (j, opts.field[j]);    \
} while (0)
          SFO  (0); SFO  (1); SFO  (2); SFO  (3);
          SFO  (4); SFO  (5); SFO  (6); SFO  (7);
          SFO  (8); SFO  (9); SFO (10); SFO (11);
#undef SFO
          
 
          if (p.seenOption ("--colorbar"))
            gwindow->scene.setColorBarOpts (opts.colorbar);

        }
  
    }
  else if ((cmd == "window") && (args.size () == 0))
    {
      std::cout << "Window list:" << std::endl;
      for (glgrib_window_set::const_iterator it = wset->begin (); it != wset->end (); it++)
        std::cout << (*it)->id () << std::endl;
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
  glgrib_shell * shell = (glgrib_shell *)data;
  shell->run ();
  return NULL;
}


void glgrib_shell::start (glgrib_window_set * ws)
{
  wset = ws;
  pthread_create(&thread, NULL, _run, this);
}


void glgrib_shell::run ()
{
  if (read_history (".glgrib_history") != 0)
    write_history (".glgrib_history");
  while (wset->size () > 0)
    {
      char * line = readline ("glgrib> ");

      lock ();

      if (line == NULL)
        {
          close = 1;
          break;
        }
      if (strlen (line) > 0) 
        add_history (line);
 
      {
        if (wset->size ())
          {
            glgrib_window * gwindow = wset->getWindowById (windowid);
	    if (gwindow == NULL)
              gwindow = wset->getFirstWindow ();
            execute (line, gwindow);
	  }
      }
      unlock ();
      
      append_history (1, ".glgrib_history");

      free (line);
      
      if (closed ()) 
        break;

    }
}


