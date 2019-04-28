#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_landscape_tex.h"
#include "glgrib_field.h"
#include "glgrib_coords_world.h"
#include "glgrib_scene.h"
#include "glgrib_grid.h"
#include "glgrib_view.h"
#include "glgrib_coastlines.h"
#include "glgrib_x11.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_options.h"

void x11_display (const glgrib_options & opts)
{
  glgrib_coastlines Coast;
  glgrib_grid Grid;
  glgrib_field Field;
  glgrib_coords_world WorldCoords;
  glgrib_landscape_tex Landscape_tex;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      return;
    }

  glgrib_window Gwindow (opts.geometry, opts.width, opts.height);
  Gwindow.scene.view.setViewport (opts.width, opts.height);
  
  gl_init ();

  WorldCoords.init (opts.geometry, opts.orography);


  if (opts.landscape)
    {
      Landscape_tex.init (&WorldCoords);
      Gwindow.scene.setLandscape (&Landscape_tex);
    }

  if (opts.grid)
    {
      Grid.init (opts.grid);
      Gwindow.scene.setGrid (&Grid);
    }

  if (opts.coasts != "")
    {
      Coast.init (opts.coasts);
      Gwindow.scene.setCoastlines (&Coast);
    }

  if (opts.field != "")
    {
      Field.init (opts.field, &WorldCoords);
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
  
}


