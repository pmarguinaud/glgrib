#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glGribScene.h"
#include "glGribShell.h"
#include "glGribWindow.h"
#include "glGribWindowSet.h"
#include "glGribWindowDiffSet.h"
#include "glGribOptions.h"
#include "glGribGeometry.h"
#include "glGribLoader.h"

#include <iostream>


namespace
{

void errorCallback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}

glGrib::WindowSet * startRegularMode (const glGrib::Options & opts)
{
  glGrib::WindowSet * wset = new glGrib::WindowSet (opts);
  wset->create (opts);
  return wset;
}

glGrib::WindowSet * startDiffMode (const glGrib::Options & opts)
{
  glGrib::WindowDiffSet * wset = new glGrib::WindowDiffSet (opts);
  return wset;
}

}

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

  glfwSetErrorCallback (errorCallback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }

  glGrib::WindowSet * wset;
  
  if (opts.diff.on)
    wset = startDiffMode (opts);
  else
    wset = startRegularMode (opts);

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

  glfwTerminate ();

  return 0;
}


