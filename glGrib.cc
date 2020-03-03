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


static void errorCallback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}

static
glGribWindowSet * startRegularMode (const glGribOptions & opts)
{
  glGribWindowSet * wset = new glGribWindowSet (opts);
  wset->create (opts);
  return wset;
}

static
glGribWindowSet * startDiffMode (const glGribOptions & opts)
{
  glGribWindowDiffSet * wset = new glGribWindowDiffSet (opts);
  return wset;
}

int main (int argc, const char * argv[])
{
  glGribOptions opts;

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

  glGribWindowSet * wset;
  
  if (opts.diff.on)
    wset = startDiffMode (opts);
  else
    wset = startRegularMode (opts);

  if (opts.shell.on)
    {
      Shell.setup (opts.shell);
      Shell.start (wset);
      wset->run (&Shell);
    }
  else
    {
      wset->run ();
    }

  Shell.wait ();

  delete wset;

  glfwTerminate ();

  return 0;
}


