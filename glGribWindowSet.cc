#include "glGribWindowSet.h"
#include "glGribWindow.h"
#include "glGribWindowOffscreen.h"


void glGribWindowSet::handleMasterWindow ()
{
  const glGribWindow * wl = nullptr;
  
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

void glGribWindowSet::runShell (glGribShell ** _shell)
{
  glGribShell * shell = *_shell;
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
          glGribWindow * w1 = w->clone ();
          insert (w1);
          break;
	}
    }

  *_shell = shell;
}


void glGribWindowSet::run (glGribShell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      runShell (&shell);
    }
}

glGribWindow * glGribWindowSet::getWindowById (int id)
{
  for (auto w : *this)
    if (w->id () == id)
      return w;
  return nullptr;
}

void glGribWindowSet::close ()
{
  for (auto win : *this)
    win->shouldClose ();
}


glGribWindow * glGribWindowSet::create (const glGribOptions & opts)
{
  glGribWindow * gwindow = nullptr;

  if (opts.window.offscreen.on)
    gwindow = new glGribWindowOffscreen (opts);
  else
    gwindow = new glGribWindow (opts);

  gwindow->scene.setup (opts);

  insert (gwindow);
  
  return gwindow;
}




