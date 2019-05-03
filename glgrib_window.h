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

  void toggle_cursorpos_display ();
  void framebuffer              ();
  void snapshot                 ();
  void toggle_flat ()
  {
    if (scene.landscape != NULL)
      scene.landscape->toggle_flat ();
  }
  void reset_view ()
  {
    glgrib_view view;
    scene.view = view;
  }
  void toggle_rotate     () { do_rotate = ! do_rotate; }
  void toggle_wireframe  () { scene.landscape->toggle_wireframe (); }
  void widen_fov         () { scene.view.fov += 1.; }
  void shrink_fov        () { scene.view.fov -= 1.; }
  void increase_radius   () { scene.view.rc += 0.1; }
  void decrease_radius   () { scene.view.rc -= 0.1; }
  void rotate_north      () { scene.view.latc = scene.view.latc + 5.; }
  void rotate_south      () { scene.view.latc = scene.view.latc - 5.; }
  void rotate_west       () { scene.view.lonc = scene.view.lonc - 5.; }
  void rotate_east       () { scene.view.lonc = scene.view.lonc + 5.; }

  void resize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  void display_cursor_position (double, double);
  int get_latlon_from_cursor (float *, float *);
};

#endif
