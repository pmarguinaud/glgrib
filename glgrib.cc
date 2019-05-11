#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_field.h"
#include "glgrib_scene.h"
#include "glgrib_view.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_window_offscreen.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <iostream>
#include <set>

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
  
  if (opts.landscape.path != "")
    gwindow->scene.landscape.init (opts);

  if (opts.grid.resolution)
    gwindow->scene.grid.init (opts);

  if (opts.coastlines.path != "")
    gwindow->scene.coastlines.init (opts);
   

  for (int i = 0; i < opts.field.size (); i++)
    {
      glgrib_field fld;
      bool defined = opts.field[i].path.size () != 0;

      if (defined)
        {
          fld.dopts.scale   = opts.field[i].scale[0];
          fld.dopts.palette = get_palette_by_name (opts.field[i].palette[0]);
          fld.init (opts.field[i].path[0], opts);
        }

      gwindow->scene.fieldlist.push_back (fld);

      if (defined)
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
  
  return 0;
}


