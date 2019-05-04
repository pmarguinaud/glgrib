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
    glgrib_view_params params;
    scene.view.params = params;
  }
  void toggle_rotate     () { do_rotate = ! do_rotate; }
  void toggle_wireframe  () { scene.landscape->toggle_wireframe (); }
  void widen_fov         () { scene.view.params.fov += 1.; }
  void shrink_fov        () { scene.view.params.fov -= 1.; }
  void increase_radius   () { scene.view.params.rc += 0.1; }
  void decrease_radius   () { scene.view.params.rc -= 0.1; }
  void rotate_north      () { scene.view.params.latc = scene.view.params.latc + 5.; }
  void rotate_south      () { scene.view.params.latc = scene.view.params.latc - 5.; }
  void rotate_west       () { scene.view.params.lonc = scene.view.params.lonc - 5.; }
  void rotate_east       () { scene.view.params.lonc = scene.view.params.lonc + 5.; }

  void resize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  void display_cursor_position (double, double);
  int get_latlon_from_cursor (float *, float *);

  void select_field (int);
  void scale_field_up ();
  void scale_field_down ();
  void toggle_hide_field ();
  void scale_palette_up ();
  void scale_palette_down ();
  void next_palette ();
  void hide_all_fields ();
  void show_all_fields ();
};

#endif
