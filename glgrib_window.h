#pragma once

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
  virtual void run (class glgrib_shell * = nullptr);
  void makeCurrent () { glfwMakeContextCurrent (window); }
  void debug (unsigned int, unsigned int, GLuint, unsigned int, int, const char *);
  int snapshot_cnt = 0;
  glgrib_scene scene;
  bool cursorpos = false;
  GLFWwindow * window = nullptr;

  void toggle_cursorpos_display ();
  void framebuffer              (const std::string & = "snapshot_%N.png");
  void snapshot                 (const std::string & = "snapshot_%N.png");
  void toggle_flat ()
  {
    scene.d.landscape.toggle_flat ();
  }
  void reset_view ()
  {
    glgrib_options_view o0;
    const glgrib_options_view & o = scene.d.view.getOptions ();
    o0.projection     = o.projection;
    o0.transformation = o.transformation;
    scene.d.view.setOptions (o0);
    scene.resize ();
  }
  void toggle_rotate       () 
  { 
    glgrib_options_scene o = scene.getSceneOptions ();
    o.rotate_earth.on = ! o.rotate_earth.on; 
    scene.setSceneOptions (o);
  }
  void toggle_rotate_light () 
  { 
    glgrib_options_scene o = scene.getSceneOptions ();
    o.light.rotate.on = ! o.light.rotate.on; 
    scene.setSceneOptions (o);
  }
  void widen_fov           () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.fov += 1.; 
    scene.d.view.setOptions (o);
  }
  void shrink_fov          () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.fov -= 1.; 
    scene.d.view.setOptions (o);
  }
  void increase_radius     () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.distance += 0.1; 
    scene.d.view.setOptions (o);
  }
  void decrease_radius     () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.distance -= 0.1; 
    scene.d.view.setOptions (o);
  }
  void rotate_north        () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.lat = o.lat + 5.; 
    scene.d.view.setOptions (o);
  }
  void rotate_south        () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.lat = o.lat - 5.; 
    scene.d.view.setOptions (o);
  }
  void rotate_west         () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.lon = o.lon - 5.; 
    scene.d.view.setOptions (o);
  }
  void rotate_east         () 
  { 
    glgrib_options_view o = scene.d.view.getOptions ();
    o.lon = o.lon + 5.; 
    scene.d.view.setOptions (o);
  }
  void toggle_wireframe ();

  void resize (int, int);
  void scroll (double, double);
  void onclick (int, int, int);
  void onkey (int, int, int, int, bool = false);
  void display_cursor_position (double, double);
  int get_latlon_from_cursor (float *, float *);
  void centerViewAtCursorPos ();
  void centerLightAtCursorPos ();
  void debugTriangleNumber ();

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
  void setCloned () { cloned = true; }
  void shouldClose () { glfwSetWindowShouldClose (window, 1); }
  
  int id () const { return id_; }

  void next_projection ();
  void toggle_transform_type ();
  void load_field (const glgrib_options_field &, int = 0);
  void remove_field (int);
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
  void setOptions (const glgrib_options_window &);
  glgrib_options_window getOptions () { return opts; }
  void startShell ()
  {
    start_shell = true;
  }
  bool getStartShell () const
  {
    return start_shell;
  }

  void fix_landscape (float, float, float, float);

  const glgrib_options_window & getOptions () const { return opts; }
protected:
  void createGFLWwindow (GLFWwindow * = nullptr);
  bool closed = false;
  bool cloned = false;
  bool master = false;
  glgrib_options_window opts;
private:
  bool start_shell = false;
  int id_;
  double t0;
  int nframes = 0;
  std::string title = "";
};

class glgrib_window_set : public std::set<glgrib_window*> 
{
public:
  void run (glgrib_shell * = nullptr);
  glgrib_window * getWindowById (int);
  glgrib_window * getFirstWindow () 
    { 
      glgrib_window_set::iterator it = begin ();
      if (it != end ())
        return *it;
      else
        return nullptr;
    }
  void close ();
};

