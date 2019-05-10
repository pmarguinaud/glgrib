#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_field.h"
#include "glgrib_scene.h"
#include "glgrib_view.h"
#include "glgrib_x11.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_window_offscreen.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <iostream>
#include <set>

void x11_display (const glgrib_options & opts)
{

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
  
  glgrib_geometry_ptr geom = glgrib_geometry_load (opts);

  if (opts.landscape.path != "")
    gwindow->scene.landscape.init (opts, geom);

  if (opts.grid.resolution)
    gwindow->scene.grid.init (opts);

  if (opts.coastlines.path != "")
    gwindow->scene.coastlines.init (opts);
   

  for (int i = 0; i < opts.field.list.size (); i++)
    {
      glgrib_field fld;

      fld.dopts.scale   = opts.field.scale[i];
      fld.dopts.palette = get_palette_by_name (opts.field.palette[i]);

      fld.init (opts.field.list[i], opts, geom);
      gwindow->scene.fieldlist.push_back (fld);

      gwindow->scene.setCurrentFieldRank (i);
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
    {
      typedef std::set<glgrib_window*> wset_t;
      wset_t wset;
      wset.insert (gwindow);

      while (! wset.empty ())
        {
          for (wset_t::iterator it = wset.begin (); it != wset.end (); it++)
            {
              glgrib_window * w = *it;
              w->run ();
              if (w->isClosed ())
	        {
                  wset.erase (w);
		  delete w;
                  break;
	        }
              if (w->isCloned ())
	        {
                  glgrib_window * w1 = w->clone ();
		  wset.insert (w1);
                  break;
	        }
	    }
	}
    }

  glfwTerminate ();
  
}


