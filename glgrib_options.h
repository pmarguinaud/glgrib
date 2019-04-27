#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <string>

class glgrib_options
{
public:
  int width= 800, height = 800;
  bool shell = false;
  std::string geometry = "";
  std::string coasts = "gshhs(3).rim";
  void parse (int argc, char * argv[]);
};

#endif
