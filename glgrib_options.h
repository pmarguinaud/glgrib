#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <string>

class glgrib_options
{
public:
  int width= 800, height = 800;
  bool shell = false;
  std::string landscape = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  int grid = 9;
  std::string geometry = "";
  std::string field = "";
  std::string coasts = "coastlines/gshhs(3).rim";
  void parse (int argc, char * argv[]);
  float orography = 0.05;
};

#endif
