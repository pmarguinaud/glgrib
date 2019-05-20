#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <string>
#include <vector>

class glgrib_options_field
{
public:
  std::vector<std::string> path;
  std::vector<std::string> palette;
  std::vector<float> scale;
  bool no_value_pointer;
};

class glgrib_options_palette
{
public:
  std::string directory = "palettes";
};

class glgrib_options_grid
{
public:
  int resolution = 9;
  int r = 255, g = 255, b = 255;
};

class glgrib_options_landscape
{
public:
  std::string path = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  float orography = 0.05;
  std::string geometry = "";
};

class glgrib_options_coastlines
{
public:
  std::string path = "coastlines/gshhs(3).rim";
  int r = 255, g = 255, b = 255;
};

class glgrib_options_window
{
public:
  int width = 800, height = 800;
  bool offscreen = false;
  int offscreen_frames = 1;
  bool statistics = false;
  std::string title = "";
};

class glgrib_options_font
{
public:
  std::string bitmap = "fonts/08.bmp";
  float scale = 0.05f;
};

class glgrib_options_light
{
public:
  bool on = false;
  bool rotate = false;
  float lon = 0.0f;
  float lat = 0.0f;
};

class glgrib_options_scene
{
public:
  bool rotate_earth = false;
  bool movie = false;
  float movie_wait = 1.0f;
  glgrib_options_light light;
};

class glgrib_options_camera
{
public:
  float distance = 6.0, lat = 0., lon = 0., fov = 20.;
};

class glgrib_options
{
public:
  std::vector<glgrib_options_field> field = 
    {glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (),
     glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field ()};
  glgrib_options_palette palette;
  glgrib_options_coastlines coastlines;
  glgrib_options_window window;
  glgrib_options_landscape landscape;
  glgrib_options_grid grid;
  glgrib_options_scene scene;
  glgrib_options_camera camera;
  glgrib_options_font font;
  bool shell = false;
  bool help = false;
  void parse (int argc, char * argv[]);
  void show_help ();
};

#endif
