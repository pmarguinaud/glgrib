#include "glGribWindowOffscreen.h"

#include <iostream>

glGrib::WindowOffscreen::WindowOffscreen (const Options & _opts) 
{
  frames = _opts.window.offscreen.frames;
  create (_opts);
  scene.setup (_opts);
  reSize (opts.width, opts.height);
}

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
