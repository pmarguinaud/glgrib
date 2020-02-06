#ifndef _GLGRIB_WINDOW_OFFSCREEN
#define _GLGRIB_WINDOW_OFFSCREEN

#include "glgrib_window.h"

class glgrib_window_offscreen : public glgrib_window
{
public:
  glgrib_window_offscreen (const glgrib_options & opts)
  {
    frames = opts.window.offscreen.frames;
    glgrib_window::create (opts);
  }
  virtual void run (class glgrib_shell * shell = NULL) override;
  virtual void setHints () override;
private:
  int frames;
};

#endif
