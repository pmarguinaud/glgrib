#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_landscape.h"
#include "glgrib_field.h"
#include "glgrib_scene.h"
#include "glgrib_grid.h"
#include "glgrib_view.h"
#include "glgrib_coastlines.h"
#include "glgrib_x11.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

void x11_display (const glgrib_options & opts)
{
  glgrib_coastlines Coast;
  glgrib_grid Grid;
  glgrib_field Field;
  glgrib_landscape Landscape;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      return;
    }

  glgrib_window Gwindow (opts);
  Gwindow.scene.view.setViewport (opts);
  
  gl_init ();

  glgrib_geometry * geom = glgrib_geometry_load (opts);

  if (opts.landscape != "")
    {
      Landscape.init (opts, geom);
      Gwindow.scene.setLandscape (&Landscape);
    }

  if (opts.grid_resolution)
    {
      Grid.init (opts);
      Gwindow.scene.setGrid (&Grid);
    }

  if (opts.coasts != "")
    {
      Coast.init (opts);
      Gwindow.scene.setCoastlines (&Coast);
    }

  if (opts.field != "")
    {
      Field.init (opts, geom);
      Gwindow.scene.setField (&Field);
    }


  if (opts.shell)
    {
#pragma omp parallel
      {
        int tid = omp_get_thread_num ();
        if (tid == 1)
          Shell.run (&Gwindow);
        else if (tid == 0)
          Gwindow.run (&Shell);
      }
    }
  else
    Gwindow.run ();

  glfwTerminate ();
  

  delete geom;

}


