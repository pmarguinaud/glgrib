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
  glgrib_window ();
  glgrib_window (const glgrib_options &);
  virtual void setHints ();
  virtual ~glgrib_window ();
  virtual void renderFrame ();
  virtual void run (class glgrib_shell * = NULL);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  void debug (unsigned int, unsigned int, GLuint, unsigned int, int, const char *);
  glgrib_options_window opts;
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  GLFWwindow * window = NULL;

  void toggle_cursorpos_display ();
  void framebuffer              (const std::string & = "snapshot_%N.png");
  void snapshot                 (const std::string & = "snapshot_%N.png");
  void toggle_flat ()
  {
    scene.d.landscape.toggle_flat ();
  }
  void reset_view ()
  {
    glgrib_options_camera o;
    scene.d.view.opts = o;
    scene.resize ();
  }
  void toggle_rotate       () { scene.d.rotate_earth       = ! scene.d.rotate_earth;       }
  void toggle_rotate_light () { scene.d.light.rotate = ! scene.d.light.rotate; }
  void toggle_wireframe    () { scene.d.landscape.toggle_wireframe (); }
  void widen_fov           () { scene.d.view.opts.fov += 1.; }
  void shrink_fov          () { scene.d.view.opts.fov -= 1.; }
  void increase_radius     () { scene.d.view.opts.distance += 0.1; }
  void decrease_radius     () { scene.d.view.opts.distance -= 0.1; }
  void rotate_north        () { scene.d.view.opts.lat = scene.d.view.opts.lat + 5.; }
  void rotate_south        () { scene.d.view.opts.lat = scene.d.view.opts.lat - 5.; }
  void rotate_west         () { scene.d.view.opts.lon = scene.d.view.opts.lon - 5.; }
  void rotate_east         () { scene.d.view.opts.lon = scene.d.view.opts.lon + 5.; }

  void resize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  void onkey (int, int, int, int, bool = false);
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
  void duplicate          ();
  void create (const glgrib_options &);

  class glgrib_window * clone ();
  bool isClosed () { return closed; }
  bool isCloned () { return cloned; }
  
  int id () const { return id_; }

  void next_projection ();
  void toggle_transform_type ();
  void load_field (const glgrib_options_field &, int = 0);
  void remove_field (int);
  void set_field_palette (const std::string &);
  void set_field_palette_min (const float);
  void set_field_palette_max (const float);
  void save_current_palette ();
  void resample_current_field ();
  void toggle_show_vector ();
  void toggle_show_norm ();
  void showHelp ();

  bool isMaster () const { return master; }
  void setMaster () { master = true; }
  void unsetMaster () { master = false; }
  void toggleMaster () { master = ! master; }
  void toggleColorBar ();

protected:
  void createGFLWwindow (GLFWwindow * = NULL);
  bool closed = false;
  bool cloned = false;
  bool master = false;
private:
  int id_;
  double t0;
  int nframes = 0;
  std::string title = "";
};

class glgrib_window_set : public std::set<glgrib_window*> 
{
public:
  void run (glgrib_shell * = NULL);
  glgrib_window * getWindowById (int);
  glgrib_window * getFirstWindow () 
    { 
      glgrib_window_set::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return NULL;
    }
};

#endif
