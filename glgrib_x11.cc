#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_landscape_tex.h"
#include "glgrib_landscape_rgb.h"
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
  glgrib_landscape_rgb Landscape_rgb;
  glgrib_landscape_tex Landscape_tex;
  char geom[opts.geometry.length () + 1];
  strcpy (geom, opts.geometry.c_str ());

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      return;
    }

  glgrib_window Gwindow (geom, opts.width, opts.height);
  Gwindow.scene.view.setViewport (opts.width, opts.height);
  
  gl_init ();

  WorldCoords.init (geom);

  if(0){
  Landscape_rgb.init (geom, &WorldCoords);
  Gwindow.scene.objlist.push_back (&Landscape_rgb);
  Gwindow.scene.landscape = &Landscape_rgb;
  }
  if(1){
  Landscape_tex.init (geom, &WorldCoords);
  Gwindow.scene.objlist.push_back (&Landscape_tex);
  Gwindow.scene.landscape = &Landscape_tex;
  }

  if(1){
  Grid.init ();
  Gwindow.scene.objlist.push_back (&Grid);
  }

  if(1){
  Coast.init (opts.coasts);
  Gwindow.scene.objlist.push_back (&Coast);
  }
  if(0){
  Field.init (geom, &WorldCoords);
  Gwindow.scene.objlist.push_back (&Field);
  Gwindow.scene.field = &Field;
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


