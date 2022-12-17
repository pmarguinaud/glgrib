#ifdef GLGRIB_USE_GLFW
#pragma once

#include "glGrib/Window.h"

namespace glGrib
{

class WindowOffscreen : public Window
{
public:
  explicit WindowOffscreen (const Options & opts);
  void run (class Shell * shell = nullptr) override;
  void setHints () override;
private:
  int frames;
};

}
#endif
