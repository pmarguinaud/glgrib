#include "glgrib_window_diff.h"

#include <iostream>



void glgrib_window_diff::onkey (int key, int scancode, int action, int mods, bool help)
{
  if ((action == GLFW_PRESS || action == GLFW_REPEAT) || help)
    {
      glgrib_window_if_key (NONE,    PAGE_UP     ,  One field forward,  next = true);
      glgrib_window_if_key (NONE,    PAGE_DOWN   ,  One field backward, prev = true);
    }
  glgrib_window::onkey (key, scancode, action, mods, help);
}

glgrib_window_diff * glgrib_window_diff_set::create_diff (const glgrib_options & opts)
{
  glgrib_window_diff * gwindow = new glgrib_window_diff (opts);
  gwindow->scene.setup (opts);
  insert (gwindow);
  return gwindow;
}

glgrib_window_diff_set::glgrib_window_diff_set (const glgrib_options & o)
{
  opts = o;

  glgrib_options opts1 = opts, opts2 = opts;

  if (opts.diff.path.size () != 2)
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

//opts1.field[0].path = opts.diff.path;
//opts1.field[0].diff.on = true;
//opts2.field[1].path.push_back (opts.diff.path[0]);


  opts1.window.position.x = 0;
  opts1.window.position.y = 0;

  opts2.window.position.x = opts1.window.position.x + opts1.window.width;
  opts2.window.position.y = 0;

  cont1 = glgrib_container::create (opts.diff.path[0], true);
  cont2 = glgrib_container::create (opts.diff.path[1], true);


  cont1->buildIndex ();
  cont2->buildIndex ();

  ext = getNextExt ();

  opts1.field[0].path.push_back (opts.diff.path[0] + "%" + ext);
  opts2.field[0].path.push_back (opts.diff.path[1] + "%" + ext);

  glgrib_window * gwindow1 = create_diff (opts1);
  glgrib_window * gwindow2 = gwindow1->clone ();

  gwindow2->setOptions (opts2.window);

  gwindow2->scene.setup (opts2);

  gwindow1->setMaster ();

  insert (gwindow2);
  
}

const std::string glgrib_window_diff_set::getNextExt () const
{
  std::string e = ext;
  while (1)
    {
      e = cont1->getNextExt (e);
      if (e == "") 
        break;
      if (cont2->hasExt (e))
        break;
      e = "";
    }
  return e;
}

void glgrib_window_diff_set::run (glgrib_shell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      glgrib_window_diff * gwindow1 = dynamic_cast<glgrib_window_diff*>(getWindowById (0));
      glgrib_window * gwindow2 = getWindowById (1);
      if ((gwindow1 != nullptr) && (gwindow2 != nullptr))
        if (gwindow1->getNext ())
          {
            ext = getNextExt ();

            if (ext != "")
              {
                // First window
                auto fopts1 = gwindow1->scene.getOptions ().field[0];
                fopts1.path.clear (); fopts1.path.push_back (cont1->getFile () + "%" + ext);
                gwindow1->makeCurrent ();
                gwindow1->scene.setFieldOptions (0, fopts1);

if(1){
                // Second window
                auto fopts2 = gwindow1->scene.getOptions ().field[0];
                fopts2.path.clear (); fopts2.path.push_back (cont2->getFile () + "%" + ext);
                gwindow2->makeCurrent ();
                gwindow2->scene.setFieldOptions (0, fopts2);
}
              }

          }
      runShell (&shell);
    }
}
