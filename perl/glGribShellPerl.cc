#include "glGribShellPerl.h"
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



glGrib::ShellPerl glGrib::ShellPerl::shellperl = glGrib::ShellPerl ();

glGrib::ShellPerl::ShellPerl ()
{
}

void glGrib::ShellPerl::process_help (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  str = do_help (args, gwindow);
}

void glGrib::ShellPerl::process_get (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  listStr = do_get (args, gwindow);
}

void glGrib::ShellPerl::process_close (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  do_close (args, gwindow);
}

void glGrib::ShellPerl::process_snapshot (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  do_snapshot (args, gwindow);
}

void glGrib::ShellPerl::process_sleep (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  do_sleep (args, gwindow);
}

void glGrib::ShellPerl::process_clone (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  do_clone (args, gwindow);
}

void glGrib::ShellPerl::process_set (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  do_set (args, gwindow);
}

void glGrib::ShellPerl::process_window (const std::vector<std::string> & args, glGrib::Window * gwindow) 
{
  listInt = do_window (args, gwindow);
}

void glGrib::ShellPerl::runWset ()
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
  glGrib::ShellPerl * shell = static_cast<glGrib::ShellPerl *>(data);
  shell->runWset ();
  return nullptr;
}

}

void glGrib::ShellPerl::start (glGrib::WindowSet * ws)
{
  pthread_create (&thread, nullptr, _run, this);
}

void glGrib::ShellPerl::run ()
{
}

void glGrib::ShellPerl::setup (const glGrib::OptionsShell & o)
{ 
  opts = o;
}

void glGrib::ShellPerl::stop ()
{
  lock ();
  wset->close ();
  unlock ();
  wait ();
}

void glGrib::ShellPerl::start (int argc, const char * argv[])
{
  gopts.parse (argc, argv);
  start (nullptr);
}





