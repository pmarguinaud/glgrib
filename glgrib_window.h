#ifndef _GLGRIB_WINDOW_H
#define _GLGRIB_WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glgrib_scene.h"
#include "glgrib_view.h"
#include "glgrib_shell.h"
#include "glgrib_options.h"

#include <string>

class glgrib_window
{
public:
  glgrib_window (const glgrib_options & opts);
  ~glgrib_window ();
  void run (class glgrib_shell * shell = NULL);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  bool do_rotate = false;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  std::string title = "";
  GLFWwindow * window;
};

#endif
