#ifdef GLGRIB_USE_GLFW
#include "glGrib/WindowOffscreen.h"

#include <iostream>

namespace glGrib
{

WindowOffscreen::WindowOffscreen (const Options & _opts) 
{
  create (_opts);
  auto cg = makeCurrent ();
  getScene ().setup (_opts);
  reSize (getOptions ().width, getOptions ().height);
  frames = _opts.render.offscreen.frames;
}

void WindowOffscreen::setHints ()
{
  auto cg = makeCurrent ();
  Window::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
}

void WindowOffscreen::run (class Shell * shell)
{
  auto cg = makeCurrent ();
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
