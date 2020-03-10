#include "glGribShellInterpreter.h"
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

glGrib::ShellInterpreter glGrib::ShellInterpreter::shellinterp = glGrib::ShellInterpreter ();

glGrib::ShellInterpreter::ShellInterpreter ()
{
}

void glGrib::ShellInterpreter::runWset ()
{
  glGrib::glfwStart ();

  wset = glGrib::WindowSet::create (gopts);
 
  hasstarted = true;

  wset->run (this);

  delete wset;

  wset = nullptr;

  glGrib::glfwStop ();
}

namespace
{

void * _run (void * data)
{
  glGrib::ShellInterpreter * shell = static_cast<glGrib::ShellInterpreter *>(data);
  shell->runWset ();
  return nullptr;
}

}

void glGrib::ShellInterpreter::start (glGrib::WindowSet * ws)
{

  if (gopts.window.offscreen.on)
    {
      hasstarted = true;
      glGrib::glfwStart ();
      wset = glGrib::WindowSet::create (gopts);
    }
  else
    {
      pthread_create (&thread, nullptr, _run, this);
      while (! hasstarted);
    }
}

void glGrib::ShellInterpreter::run ()
{
}

void glGrib::ShellInterpreter::setup (const glGrib::OptionsShell & o)
{ 
  opts = o;
}

void glGrib::ShellInterpreter::stop (const std::vector <std::string> & args)
{
  unlock ();

  wset->close ();

  if (gopts.window.offscreen.on)
    {
      glGrib::glfwStop ();
    }
  else
    {
      wait ();
    }
    
  wset = nullptr;
  hasstarted = false;
}

void glGrib::ShellInterpreter::start (const std::vector <std::string> & args)
{
  int argc = args.size ();
  const char * argv[argc];
  for (int i = 0; i < args.size (); i++)
    argv[i] = args[i].c_str ();
  start (argc, argv);
}

void glGrib::ShellInterpreter::start (int argc, const char * argv[])
{
  gopts.parse (argc, argv);
  start (nullptr);
}

void glGrib::ShellInterpreter::execute (const std::vector<std::string> & args)
{
  if (args[0] == "start")
    return start (args);
  lock ();
  if (args[0] == "stop")
    return stop (args);
  glGrib::Shell::execute (args);
  unlock ();
}

