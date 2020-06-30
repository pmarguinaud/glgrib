#include "glGribWindowOffscreen.h"

#include <iostream>

glGrib::WindowOffscreen::WindowOffscreen (const Options & _opts) 
{
  create (_opts);
  getScene ().setup (_opts);
  reSize (getOptions ().width, getOptions ().height);
  frames = _opts.render.offscreen.frames;
}

void glGrib::WindowOffscreen::setHints ()
{
#ifdef USE_GLFW
  glGrib::Window::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
#endif
}

void glGrib::WindowOffscreen::run (class glGrib::Shell * shell)
{
  const auto & opts = getOptions ();
  for (int i = 0; i < frames; i++)
    {
      update ();
      framebuffer (opts.offscreen.format);
    }
  close ();
}
