#pragma once

#include "glGribWindow.h"

namespace glGrib
{

class WindowOffscreen : public Window
{
public:
  WindowOffscreen (const Options & opts) : Window (opts)
  {
    frames = opts.window.offscreen.frames;
  }
  void run (class Shell * shell = nullptr) override;
  void setHints () override;
private:
  int frames;
};


}
