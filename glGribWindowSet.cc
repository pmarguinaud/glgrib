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

void glGribWindowSet::updateWindows ()
{
  for (auto w : *this)
    {
      glGribField * f = w->scene.getCurrentField ();

      if (f == nullptr)
        continue;

      w->makeCurrent ();

      int d = 0.0f;

      if (w->getNext ())
        d = +1;
      if (w->getPrev ())
        d = -1;

      if (d == 0)
        continue;

      int rank = w->scene.getCurrentFieldRank ();
      auto fopts = opts.field[rank];

      float slot = std::max (0.0f, std::min (float (f->getSlotMax ()-1), f->getSlot ()+d));

      w->scene.setFieldOptions (rank, fopts, slot);

    }
}

void glGribWindowSet::run (glGribShell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      updateWindows ();
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

glGribWindowSet::glGribWindowSet (const glGribOptions & o)
{
  opts = o;
}


