#include "glGrib/ShellInterpreter.h"
#include "glGrib/WindowSet.h"

#include <iostream>
#include <exception>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>

#include <unistd.h>

namespace glGrib
{

ShellInterpreter * ShellInterpreter::shellinterp = nullptr;

ShellInterpreter::ShellInterpreter ()
{
}

void ShellInterpreter::lock () 
{ 
  mutex.lock (); 
}

void ShellInterpreter::unlock () 
{ 
#ifdef GLGRIB_USE_GLFW
  glfwMakeContextCurrent (nullptr);
#endif
  mutex.unlock (); 
}

void ShellInterpreter::runWset ()
{
  glStart (gopts.render);

  setWindowSet (WindowSet::create (gopts));
 
  hasstarted = true;

  getWindowSet ()->run (this);

  clearWindowSet ();

  glStop ();
}

void ShellInterpreter::start (WindowSet * ws)
{
  if (gopts.render.offscreen.on)
    {
      hasstarted = true;
      glStart (gopts.render);
      setWindowSet (WindowSet::create (gopts));
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

void ShellInterpreter::run ()
{
}

void ShellInterpreter::setup (const OptionsShell & o)
{ 
  setOptions (o);
}

void ShellInterpreter::stop (const std::vector <std::string> & args)
{
  if (closed ())
    return;

  unlock ();

  getWindowSet ()->close ();

  if (gopts.render.offscreen.on)
    {
      glStop ();
    }
  else
    {
      wait ();
    }
    
  clearWindowSet ();
  hasstarted = false;
}

void ShellInterpreter::start (const std::vector <std::string> & args)
{
  int argc = args.size ();
  const char * argv[argc];
  for (size_t i = 0; i < args.size (); i++)
    argv[i] = args[i].c_str ();
  start (argc, argv);
}

void ShellInterpreter::start (int argc, const char * argv[])
{
  gopts.parse (argc, argv);
  start (nullptr);
}

void ShellInterpreter::execute (const std::vector<std::string> & args)
{
  if (closed ())
    return;
  if (args[0] == "start")
    return start (args);
  lock ();
  if (args[0] == "stop")
    return stop (args);
  Shell::execute (args);
  unlock ();
}

}
