#include "glGribWindowOffscreen.h"

#include <iostream>

void glGribWindowOffscreen::setHints ()
{
  glGribWindow::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
}

void glGribWindowOffscreen::run (class glGribShell * shell)
{
  for (int i = 0; i < frames; i++)
    {
      scene.update ();
      framebuffer (opts.offscreen.format);
    }
  closed = true;
}
