#ifdef USE_GLFW
#include "glGribWindowOffscreen.h"

#include <iostream>

namespace glGrib
{

WindowOffscreen::WindowOffscreen (const Options & _opts) 
{
  create (_opts);
  getScene ().setup (_opts);
  reSize (getOptions ().width, getOptions ().height);
  frames = _opts.render.offscreen.frames;
}

void WindowOffscreen::setHints ()
{
  Window::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
}

void WindowOffscreen::run (class Shell * shell)
{
  const auto & opts = getOptions ();
  for (int i = 0; i < frames; i++)
    {
      update ();
      framebuffer (opts.offscreen.format);
    }
  close ();
}

}
#endif
