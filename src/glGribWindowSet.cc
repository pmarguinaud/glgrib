#include "glGribWindowSet.h"
#include "glGribWindowDiffSet.h"
#include "glGribWindowReviewSet.h"
#include "glGribRender.h"
#include "glGribWindow.h"
#include "glGribBatch.h"
#include "glGribWindowOffscreen.h"
#include "glGribShellRegular.h"


void glGrib::WindowSet::handleMasterWindow ()
{
  const glGrib::Render * wl = nullptr;
  
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

void glGrib::WindowSet::runShell (glGrib::Shell ** _shell)
{
  glGrib::Shell * shell = *_shell;
  for (auto w : *this)
    {
      if ((! shell) && (w->getStartShell ()))
        {
          shell = &glGrib::ShellRegular::getInstance ();
          shell->setup (w->getScene ().getOptions ().shell);
          shell->start (this);
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
          glGrib::Render * w1 = w->clone ();
          insert (w1);
          break;
	}
    }

  *_shell = shell;
}

void glGrib::WindowSet::updateWindows ()
{
  for (auto w : *this)
    {
      glGrib::Field * f = w->getScene ().getCurrentField ();

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

void glGrib::WindowSet::run (glGrib::Shell * shell)
{
  while (! empty ())
    {
      handleMasterWindow ();
      updateWindows ();
      runShell (&shell);
    }
}

glGrib::Render * glGrib::WindowSet::getWindowById (int id)
{
  for (auto w : *this)
    if (w->id () == id)
      return w;
  return nullptr;
}

void glGrib::WindowSet::close ()
{
  for (auto win : *this)
    win->shouldClose ();
}


glGrib::Render * glGrib::WindowSet::createWindow (const glGrib::Options & opts)
{
  glGrib::Render * gwindow = nullptr;

#ifdef USE_GLFW
  if (opts.render.offscreen.on)
    gwindow = new glGrib::WindowOffscreen (opts);
  else
    gwindow = new glGrib::Window (opts);
#endif
#ifdef USE_EGL
  gwindow = new glGrib::Batch (opts);
#endif

  insert (gwindow);
  
  return gwindow;
}

glGrib::WindowSet::WindowSet (const glGrib::Options & o, bool newwin)
{
  opts = o;
  if (newwin)
    createWindow (opts);
}

glGrib::WindowSet * glGrib::WindowSet::create (const glGrib::Options & opts)
{
  if (opts.diff.on)
    return new glGrib::WindowDiffSet (opts);
  if (opts.review.on)
    return new glGrib::WindowReviewSet (opts);
  return new glGrib::WindowSet (opts);
}


