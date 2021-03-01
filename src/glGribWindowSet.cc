#include "glGribWindowSet.h"
#include "glGribWindowDiffSet.h"
#include "glGribWindowReviewSet.h"
#include "glGribRender.h"
#include "glGribWindow.h"
#include "glGribBatch.h"
#include "glGribWindowOffscreen.h"
#include "glGribShellRegular.h"


namespace glGrib
{

void WindowSet::handleMasterWindow ()
{
  const Render * wl = nullptr;
  
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
      w->getScene ().setViewOptions (wl->getScene ().getViewOptions ());
}

void WindowSet::runShell (Shell ** _shell, bool render)
{
  Shell * shell = *_shell;
  for (auto w : *this)
    {
      if ((! shell) && (w->getStartShell ()))
        {
          shell = &ShellRegular::getInstance ();
          shell->setup (w->getScene ().getOptions ().shell);
          shell->start (this);
        }
  
      if (render)
        w->run (shell);
  
      if (w->isClosed ())
        {
          erase (w);
          delete w;
          break;
        }
  
      if (w->isCloned ())
        {
          Render * w1 = w->clone ();
          insert (w1);
          break;
	}
      if (w->isCleared ())
        {
          w->clear ();
        }
    }

  *_shell = shell;
}

void WindowSet::updateWindows ()
{
  for (auto w : *this)
    {
      Field * f = w->getScene ().getCurrentField ();

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

      int rank = w->getScene ().getCurrentFieldRank ();
      auto fopts = opts.field[rank];

      float slot = std::max (0.0f, std::min (float (f->getSlotMax ()-1), f->getSlot ()+d));

      w->getScene ().setFieldOptions (rank, fopts, slot);

    }
}

void WindowSet::run (Shell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      updateWindows ();
      runShell (&shell);
    }
}

Render * WindowSet::getWindowById (int id)
{
  for (auto w : *this)
    if (w->id () == id)
      return w;
  return nullptr;
}

void WindowSet::close ()
{
  for (auto win : *this)
    win->shouldClose ();
}


Render * WindowSet::createWindow (const Options & opts)
{
  Render * gwindow = nullptr;

#ifdef GLGRIB_USE_GLFW
  if (opts.render.offscreen.on)
    gwindow = new WindowOffscreen (opts);
  else
    gwindow = new Window (opts);
#endif
#ifdef GLGRIB_USE_EGL
  gwindow = new Batch (opts);
#endif

  insert (gwindow);
  
  return gwindow;
}

WindowSet::WindowSet (const Options & o, bool newwin)
{
  opts = o;
  if (newwin)
    createWindow (opts);
}

WindowSet * WindowSet::create (const Options & opts)
{
#ifdef GLGRIB_USE_GLFW
  if (opts.diff.on)
    return new WindowDiffSet (opts);
  if (opts.review.on)
    return new WindowReviewSet (opts);
#endif
  return new WindowSet (opts);
}

}
