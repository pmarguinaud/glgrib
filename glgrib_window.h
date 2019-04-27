#ifndef _GLGRIB_WINDOW_H
#define _GLGRIB_WINDOW_H

#include "glgrib_view.h"
#include "glgrib_scene.h"
#include "glgrib_shell.h"

class glgrib_window
{
public:
  glgrib_window (const char *, int, int);
  ~glgrib_window ();
  void run (class glgrib_shell * shell = NULL);
  bool do_rotate = false;
  glgrib_view * view;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene * scene;
  bool cursorpos = false;
  const char * title = "";
  GLFWwindow * window;
};

#endif
