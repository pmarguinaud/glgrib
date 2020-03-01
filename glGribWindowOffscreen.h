#pragma once

#include "glGribWindow.h"

class glGribWindowOffscreen : public glGribWindow
{
public:
  glGribWindowOffscreen (const glGribOptions & opts)
  {
    frames = opts.window.offscreen.frames;
    glGribWindow::create (opts);
  }
  void run (class glGribShell * shell = nullptr) override;
  void setHints () override;
private:
  int frames;
};

