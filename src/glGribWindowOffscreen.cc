#include "glGribWindowOffscreen.h"

#include <iostream>

void glGrib::WindowOffscreen::setHints ()
{
  glGrib::Window::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
}

void glGrib::WindowOffscreen::run (class glGrib::Shell * shell)
{
  for (int i = 0; i < frames; i++)
    {
      scene.update ();
      framebuffer (opts.offscreen.format);
    }
  closed = true;
}
