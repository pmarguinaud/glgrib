#include "glGribShellInterpreter.h"
#include "glGribWindowSet.h"
#include "glGribWindow.h"

#include <iostream>
#include <exception>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>

#include <unistd.h>

glGrib::ShellInterpreter * glGrib::ShellInterpreter::shellinterp = nullptr;

glGrib::ShellInterpreter::ShellInterpreter ()
{
}

void glGrib::ShellInterpreter::runWset ()
{
  glGrib::glStart ();

  setWindowSet (glGrib::WindowSet::create (gopts));
 
  hasstarted = true;

  getWindowSet ()->run (this);

  clearWindowSet ();

  glGrib::glStop ();
}

void glGrib::ShellInterpreter::start (glGrib::WindowSet * ws)
{
  if (gopts.window.offscreen.on)
    {
      hasstarted = true;
      glGrib::glStart ();
      setWindowSet (glGrib::WindowSet::create (gopts));
    }
  else
    {
      thread = std::thread ([this] () 
      { 
        this->runWset ();
        this->lock ();
        this->setClosed ();
        this->unlock ();
      });
      while (! hasstarted);
    }
}

void glGrib::ShellInterpreter::run ()
{
}

void glGrib::ShellInterpreter::setup (const glGrib::OptionsShell & o)
{ 
  setOptions (o);
}

void glGrib::ShellInterpreter::stop (const std::vector <std::string> & args)
{
  if (closed ())
    return;

  unlock ();

  getWindowSet ()->close ();

  if (gopts.window.offscreen.on)
    {
      glGrib::glStop ();
    }
  else
    {
      wait ();
    }
    
  clearWindowSet ();
  hasstarted = false;
}

void glGrib::ShellInterpreter::start (const std::vector <std::string> & args)
{
  int argc = args.size ();
  const char * argv[argc];
  for (size_t i = 0; i < args.size (); i++)
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
  if (closed ())
    return;
  if (args[0] == "start")
    return start (args);
  lock ();
  if (args[0] == "stop")
    return stop (args);
  glGrib::Shell::execute (args);
  unlock ();
}

