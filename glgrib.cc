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

int main (int argc, char * argv[])
{
  glgrib_options opts;
  opts.parse (argc, argv);  

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      exit (EXIT_FAILURE);
    }

  palette_directory = opts.palette.directory;

  glgrib_window * gwindow;

  if (opts.window.offscreen)
    gwindow = new glgrib_window_offscreen (opts);
  else
    gwindow = new glgrib_window (opts);
  
  gwindow->scene.init (opts);

  glgrib_window_set wset;
  wset.insert (gwindow);

  if (opts.shell)
    {
      Shell.start (&wset);
      wset.run (&Shell);
      Shell.wait ();
    }
  else
    wset.run ();
   

  glfwTerminate ();
  
  return 0;
}


