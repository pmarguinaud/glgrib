#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glGribShellRegular.h"
#include "glGribWindowSet.h"
#include "glGribWindowDiffSet.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"

#include <iostream>


int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if ((argc == 2) && strncmp (argv[1], "--", 2))
    {
      opts.shell.on = true;
      opts.shell.script = std::string (argv[1]);
    }
  else if (! opts.parse (argc, argv))
    return 1;

  glGrib::glfwStart ();

  glGrib::WindowSet * wset = glGrib::WindowSet::create (opts);
  glGrib::Shell & shell = glGrib::ShellRegular::getInstance ();

  if (opts.shell.on)
    {
      shell.setup (opts.shell);
      shell.start (wset);
      wset->run (&shell);
    }
  else
    {
      wset->run ();
    }

  shell.wait ();

  delete wset;

  glGrib::glfwStop ();

  return 0;
}


