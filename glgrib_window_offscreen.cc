#include "glgrib_window_offscreen.h"

#include <iostream>

void glgrib_window_offscreen::setHints ()
{
  glgrib_window::setHints ();
  glfwWindowHint (GLFW_VISIBLE, GLFW_FALSE);
}

void glgrib_window_offscreen::run (class glgrib_shell * shell)
{
  renderFrame ();
  framebuffer ();
}
