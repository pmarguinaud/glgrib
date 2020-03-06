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

  glGrib::WindowSet * wset = opts.diff.on
      ? new glGrib::WindowDiffSet (opts)
      : new glGrib::WindowSet (opts);

  if (opts.shell.on)
    {
      glGrib::Shell0.setup (opts.shell);
      glGrib::Shell0.start (wset);
      wset->run (&glGrib::Shell0);
    }
  else
    {
      wset->run ();
    }

  glGrib::Shell0.wait ();

  delete wset;

  glGrib::glfwStop ();

  return 0;
}


