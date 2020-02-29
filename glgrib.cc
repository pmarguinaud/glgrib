#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_scene.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_window_set.h"
#include "glgrib_window_diff.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_loader.h"

#include <iostream>


static void error_callback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}

static
glgrib_window_set * startRegularMode (const glgrib_options & opts)
{
  glgrib_window_set * wset = new glgrib_window_set ();
  wset->create (opts);
  return wset;
}

static
glgrib_window_set * startDiffMode (const glgrib_options & opts)
{
  glgrib_window_diff_set * wset = new glgrib_window_diff_set (opts);
  return wset;
}

int main (int argc, const char * argv[])
{
  glgrib_options opts;

  if ((argc == 2) && strncmp (argv[1], "--", 2))
    {
      opts.shell.on = true;
      opts.shell.script = std::string (argv[1]);
    }
  else if (! opts.parse (argc, argv))
    return 1;

  glfwSetErrorCallback (error_callback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }

  glgrib_window_set * wset;
  
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


