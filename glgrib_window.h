#ifndef _GLGRIB_WINDOW_H
#define _GLGRIB_WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glgrib_scene.h"
#include "glgrib_view.h"
#include "glgrib_shell.h"

class glgrib_window
{
public:
  glgrib_window (const char *, int, int);
  ~glgrib_window ();
  void run (class glgrib_shell * shell = NULL);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  bool do_rotate = false;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  const char * title = "";
  GLFWwindow * window;
};

#endif
