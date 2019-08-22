#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_scene.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_window_offscreen.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <iostream>


static void error_callback (int c, const char * desc)
{
  fprintf (stderr, "Error: %4d | %s\n", c, desc);
  abort ();
}

int main (int argc, const char * argv[])
{
  glgrib_options opts;
  if (! opts.parse (argc, argv))
    return 0;

  glfwSetErrorCallback (error_callback);

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }

  palette_directory = opts.palette.directory;

  glgrib_window * gwindow;

  if (opts.window.offscreen.on)
    gwindow = new glgrib_window_offscreen (opts);
  else
    gwindow = new glgrib_window (opts);

  gwindow->scene.init (opts);
  
  glgrib_window_set wset;
  wset.insert (gwindow);

  if (opts.shell.on)
    {
      Shell.start (&wset);
      wset.run (&Shell);
      Shell.wait ();
    }
  else
    {
      wset.run ();
    }
   

  glfwTerminate ();
  
  return 0;
}


