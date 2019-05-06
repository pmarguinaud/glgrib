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
#include "glgrib_window_offscreen.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <iostream>

void x11_display (const glgrib_options & opts)
{
  glgrib_coastlines Coast;
  glgrib_grid Grid;
  glgrib_field Field[10];
  glgrib_landscape Landscape;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      return;
    }

  glgrib_window * gwindow;

  if (opts.window.offscreen)
    gwindow = new glgrib_window_offscreen (opts);
  else
    gwindow = new glgrib_window (opts);
  
  gwindow->scene.view.setViewport (opts);
  
  gl_init ();

  glgrib_geometry_ptr geom = glgrib_geometry_load (opts);

  if (opts.landscape.path != "")
    {
      Landscape.init (opts, geom);
      gwindow->scene.setLandscape (&Landscape);
    }

  if (opts.grid.resolution)
    {
      Grid.init (opts);
      gwindow->scene.setGrid (&Grid);
    }

  if (opts.coastlines.path != "")
    {
      Coast.init (opts);
      gwindow->scene.setCoastlines (&Coast);
    }

  for (int i = 0; i < opts.field.list.size (); i++)
    {
      Field[i].init (opts.field.list[i], opts, geom);
      
      gwindow->scene.setField (&Field[i]);
      gwindow->scene.fieldlist[i] = &Field[i];

      gwindow->scene.fieldoptslist[i].scale = opts.field.scale[i];
      gwindow->scene.fieldoptslist[i].palette = get_palette_by_name (opts.field.palette[i]);
      gwindow->scene.currentFieldOpts = &gwindow->scene.fieldoptslist[i];
    }


  if (opts.shell)
    {
#pragma omp parallel
      {
        int tid = omp_get_thread_num ();
        if (tid == 1)
          Shell.run (gwindow);
        else if (tid == 0)
          gwindow->run (&Shell);
      }
    }
  else
    gwindow->run ();

  glfwTerminate ();
  

  delete gwindow;
 
}


