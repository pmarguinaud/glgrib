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
  virtual ~glgrib_window ();
  void run (class glgrib_shell * shell = NULL);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  bool do_rotate = false;
  int width, height;
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  std::string title = "";
  GLFWwindow * window;
  bool left_shift = false;
  bool right_shift = false;
  bool left_control = false;
  bool right_control = false;
  bool left_alt = false;
  bool right_alt = false;

  void toggle_cursorpos_display ();
  void toggle_rotate            ();
  void toggle_wireframe         ();
  void framebuffer              ();
  void widen_fov                ();
  void snapshot                 ();
  void shrink_fov               ();
  void toggle_flat              ();
  void increase_radius          ();
  void decrease_radius          ();
  void reset_view               ();
  void rotate_north             ();
  void rotate_south             ();
  void rotate_west              ();
  void rotate_east              ();
  void set_left_shift           ();
  void set_right_shift          ();
  void set_left_control         ();
  void set_right_control        ();
  void set_left_alt             ();
  void set_right_alt            ();
};

#endif
