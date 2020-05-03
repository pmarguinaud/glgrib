#include "glGribShellRegular.h"
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



glGrib::ShellRegular glGrib::ShellRegular::shellregular = glGrib::ShellRegular ();

char * glGrib::ShellRegular::optionGenerator (const char * text, int state)
{

  if (! state) 
    {
      og.list_index = 0;
      og.text_len = strlen (text);
    }

  std::string tt = text;

  for (; static_cast<size_t> (og.list_index) < getsetoptions.size (); 
       og.list_index++)
    if (getsetoptions[og.list_index].substr (0, og.text_len) == tt)
      return strdup (getsetoptions[og.list_index++].c_str ());  // Note the ++

  return nullptr;
}


namespace
{

char * shellOptionGenerator (const char * text, int state)
{
  return glGrib::ShellRegular::getInstance ().optionGenerator (text, state);
}

char ** shellCompletion (const char * text, int start, int end)
{
  char * line = rl_line_buffer;
  if ((strncmp (line, "set ", 4) == 0) || (strncmp (line, "get ", 4) == 0))
    return rl_completion_matches (text, shellOptionGenerator);
  return nullptr;
}

}

glGrib::ShellRegular::ShellRegular ()
{
  glGrib::Options opts;
  glGrib::OptionsParser p;
  opts.traverse ("", &p);
  p.getOptions (&getsetoptions);
  rl_attempted_completion_function = shellCompletion;
}

void glGrib::ShellRegular::process_help (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  for (const auto & h : listStr)
    std::cout << h;
}

void glGrib::ShellRegular::process_get (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  for (const auto & o : listStr)
     std::cout << o << " ";
  std::cout << std::endl;
}

void glGrib::ShellRegular::process_json (const std::vector<std::string> & args, glGrib::Window * gwindow)
{
  for (const auto & o : listStr)
     std::cout << o << " ";
  std::cout << std::endl;
}

void glGrib::ShellRegular::process_window (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  if (listStr.size () > 0)
    {
      std::cout << "Window list:" << std::endl;
      for (const auto id : listStr)
        std::cout << (std::to_string (windowid) == id ? " > " : "   ") << id << std::endl;
    }
}

std::vector<std::string> glGrib::ShellRegular::tokenize (const std::string & _line)
{
  std::vector<std::string> args;

  std::string line = _line;

  try 
    {
      args.push_back (glGrib::OptionsUtil::nextToken (&line));

      if ((args[0] == "") || (args[0][0] == '#'))
        {
          args.clear ();
          return args;
        }
     
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
      args.clear ();
      return args;
    }

  return args;
}

namespace
{

void * _run (void * data)
{
  glGrib::ShellRegular * shell = static_cast<glGrib::ShellRegular *>(data);
  shell->run ();
  return nullptr;
}

}

void glGrib::ShellRegular::start (glGrib::WindowSet * ws)
{
  wset = ws;
  pthread_create (&thread, nullptr, _run, this);
}


void glGrib::ShellRegular::runInt ()
{
  if (read_history (".glGribHistory") != 0)
    write_history (".glGribHistory");
  while (wset->size () > 0)
    {
      char * line = readline ("glGrib> ");

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
              execute (tokenize (line));
	  }
      }
      unlock ();
      
      append_history (1, ".glGribHistory");

      free (line);
      
      if (closed ()) 
        break;

    }
}

void glGrib::ShellRegular::runOff ()
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
            execute (tokenize (line));
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

void glGrib::ShellRegular::run ()
{
  if (opts.script != "")
    runOff ();
  else
    runInt ();
}

void glGrib::ShellRegular::setup (const glGrib::OptionsShell & o)
{ 
  opts = o;
}




