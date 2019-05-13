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
};

class glgrib_options_window
{
public:
  int width = 800, height = 800;
  bool offscreen = false;
  int offscreen_frames = 1;
};

class glgrib_options_scene
{
public:
  bool rotate_earth = false;
  bool rotate_light = false;
  bool light = false;
  bool movie = false;
  float movie_wait = 1.0f;
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
  bool shell = false;
  bool help = false;
  void parse (int argc, char * argv[]);
  void show_help ();
};

#endif
