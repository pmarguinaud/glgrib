#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_scene.h"
#include "glgrib_shell.h"
#include "glgrib_window.h"
#include "glgrib_window_set.h"
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
glgrib_window_set * startDiffMode (const glgrib_options & _opts)
{
  glgrib_window_set * wset = new glgrib_window_set ();

  glgrib_options opts1 = _opts, opts2 = _opts;

  if (_opts.diff.path.size () != 2)
    throw std::runtime_error (std::string ("Option --diff.path requires two arguments"));


  auto fixOpts = [] (glgrib_options * opts)
  {
    opts->diff.on = false;
    opts->diff.path.clear ();
    for (int i = 0; i < opts->field.size (); i++)
      opts->field[i].path.clear ();
  };

  fixOpts (&opts1);
  fixOpts (&opts2);

//opts1.field[0].path = _opts.diff.path;
//opts1.field[0].diff.on = true;
//opts2.field[1].path.push_back (_opts.diff.path[0]);


  opts1.window.position.x = 0;
  opts1.window.position.y = 0;

  opts2.window.position.x = opts1.window.position.x + opts1.window.width;
  opts2.window.position.y = 0;

  glgrib_container * cont1 = glgrib_container::create (_opts.diff.path[0], true);
  glgrib_container * cont2 = glgrib_container::create (_opts.diff.path[1], true);

  cont1->buildIndex ();
  cont2->buildIndex ();

  std::string ext;
  for (auto it = cont1->begin (); it != cont1->end (); ++it)
    {
      ext = *it;
      if (cont2->hasExt (ext))
        break;
      ext = "";
    }

  std::cout << " ext = " << ext << std::endl;

  opts1.field[0].path.push_back (_opts.diff.path[0] + "%" + ext);
  opts2.field[0].path.push_back (_opts.diff.path[1] + "%" + ext);

  glgrib_window * gwindow1 = wset->create (opts1);
  glgrib_window * gwindow2 = gwindow1->clone ();

  gwindow2->setOptions (opts2.window);

  gwindow2->scene.setup (opts2);

  gwindow1->setMaster ();

  wset->insert (gwindow2);
  
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


