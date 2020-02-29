#include "glgrib_window_set.h"
#include "glgrib_window.h"
#include "glgrib_window_offscreen.h"


void glgrib_window_set::handleMasterWindow ()
{
  const glgrib_window * wl = nullptr;
  
  for (auto w : *this)
    if (w->isMaster ())
      {
        if (wl != nullptr)
          w->unsetMaster ();
        else
          wl = w;
      }
  if (wl != nullptr)
    for (auto w : *this)
      w->scene.d.view.setOptions (wl->scene.d.view.getOptions ());
}

void glgrib_window_set::runShell (glgrib_shell ** _shell)
{
  glgrib_shell * shell = *_shell;
  for (auto w : *this)
    {
      if ((! shell) && (w->getStartShell ()))
        {
          Shell.setup (w->scene.getOptions ().shell);
          Shell.start (this);
          shell = &Shell;
        }
  
      w->run (shell);
  
      if (w->isClosed ())
        {
          erase (w);
          delete w;
          break;
        }
  
      if (w->isCloned ())
        {
          glgrib_window * w1 = w->clone ();
          insert (w1);
          break;
	}
    }

  *_shell = shell;
}


void glgrib_window_set::run (glgrib_shell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      runShell (&shell);
    }
}

glgrib_window * glgrib_window_set::getWindowById (int id)
{
  for (auto w : *this)
    if (w->id () == id)
      return w;
  return nullptr;
}

void glgrib_window_set::close ()
{
  for (auto win : *this)
    win->shouldClose ();
}


glgrib_window * glgrib_window_set::create (const glgrib_options & opts)
{
  glgrib_window * gwindow = nullptr;

  if (opts.window.offscreen.on)
    gwindow = new glgrib_window_offscreen (opts);
  else
    gwindow = new glgrib_window (opts);

  gwindow->scene.setup (opts);

  insert (gwindow);
  
  return gwindow;
}




