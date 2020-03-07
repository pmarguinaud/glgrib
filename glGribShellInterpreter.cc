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

glGrib::ShellInterpreter glGrib::ShellInterpreter::shellperl = glGrib::ShellInterpreter ();

glGrib::ShellInterpreter::ShellInterpreter ()
{
}

void glGrib::ShellInterpreter::runWset ()
{
  glGrib::glfwStart ();

  wset = glGrib::WindowSet::create (gopts);

  wset->run (this);

  delete wset;

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
  pthread_create (&thread, nullptr, _run, this);
}

void glGrib::ShellInterpreter::run ()
{
}

void glGrib::ShellInterpreter::setup (const glGrib::OptionsShell & o)
{ 
  opts = o;
}

void glGrib::ShellInterpreter::stop ()
{
  lock ();
  wset->close ();
  unlock ();
  wait ();
}

void glGrib::ShellInterpreter::start (int argc, const char * argv[])
{
  gopts.parse (argc, argv);
  start (nullptr);
}

