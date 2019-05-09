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
  glgrib_window () {}
  glgrib_window (const glgrib_options &);
  virtual void setHints ();
  virtual ~glgrib_window ();
  virtual void renderFrame ();
  virtual void run (class glgrib_shell * = NULL);
  void makeCurrent () { glfwMakeContextCurrent (window); }

  int width = 0, height = 0;
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  std::string title = "";
  GLFWwindow * window = NULL;

  void toggle_cursorpos_display ();
  void framebuffer              ();
  void snapshot                 ();
  void toggle_flat ()
  {
    scene.landscape.toggle_flat ();
  }
  void reset_view ()
  {
    glgrib_view_params params;
    scene.view.params = params;
  }
  void toggle_rotate       () { scene.rotate_earth       = ! scene.rotate_earth;       }
  void toggle_rotate_light () { scene.rotate_light = ! scene.rotate_light; }
  void toggle_wireframe    () { scene.landscape.toggle_wireframe (); }
  void widen_fov           () { scene.view.params.fov += 1.; }
  void shrink_fov          () { scene.view.params.fov -= 1.; }
  void increase_radius     () { scene.view.params.rc += 0.1; }
  void decrease_radius     () { scene.view.params.rc -= 0.1; }
  void rotate_north        () { scene.view.params.latc = scene.view.params.latc + 5.; }
  void rotate_south        () { scene.view.params.latc = scene.view.params.latc - 5.; }
  void rotate_west         () { scene.view.params.lonc = scene.view.params.lonc - 5.; }
  void rotate_east         () { scene.view.params.lonc = scene.view.params.lonc + 5.; }

  void resize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  void onkey (int, int, int, int);
  void display_cursor_position (double, double);
  int get_latlon_from_cursor (float *, float *);
  void centerViewAtCursorPos ();
  void centerLightAtCursorPos ();

  void select_field (int);
  void scale_field_up ();
  void scale_field_down ();
  void toggle_hide_field ();
  void scale_palette_up ();
  void scale_palette_down ();
  void next_palette ();
  void hide_all_fields ();
  void show_all_fields ();
  void toggle_light ();
  void rotate_light_north ();
  void rotate_light_south ();
  void rotate_light_west  ();
  void rotate_light_east  ();
  void movie              ();
  void duplicate          ();
  void create (const glgrib_options &);

  class glgrib_window * clone ();
  bool isClosed () { return closed; }
  bool isCloned () { return cloned; }

protected:
  void createGFLWwindow (GLFWwindow * = NULL);
  bool closed = false;
  bool cloned = false;
};

#endif
