#pragma once

#include "glgrib_window.h"

class glgrib_window_offscreen : public glgrib_window
{
public:
  glgrib_window_offscreen (const glgrib_options & opts)
  {
    frames = opts.window.offscreen.frames;
    glgrib_window::create (opts);
  }
  void run (class glgrib_shell * shell = NULL) override;
  void setHints () override;
private:
  int frames;
};

