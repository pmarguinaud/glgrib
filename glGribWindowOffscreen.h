#pragma once

#include "glGribWindow.h"

namespace glGrib
{

class WindowOffscreen : public Window
{
public:
  WindowOffscreen (const Options & opts)
  {
    frames = opts.window.offscreen.frames;
    Window::create (opts);
  }
  void run (class Shell * shell = nullptr) override;
  void setHints () override;
private:
  int frames;
};


}
