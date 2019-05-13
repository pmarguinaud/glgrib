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
  
  gwindow->scene.init (opts);

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


