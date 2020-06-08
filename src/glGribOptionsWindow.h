#pragma once

#include "glGribOptionsBase.h"
#include "glGribOptionsOffscreen.h"

namespace glGrib
{

class OptionsWindow : public OptionsBase
{
public:
  DEFINE
  {
    DESC (width,                 Window width);
    DESC (height,                Window height);
    DESC (statistics.on,         Issue statistics when window is closed);
    DESC (antialiasing.on,       Enable antialiasing);
    DESC (antialiasing.samples,  Samples for antialiasing);
    DESC (title,                 Window title);
    DESC (debug.on,              Enable OpenGL debugging);
    DESC (version_major,         GLFW_CONTEXT_VERSION_MAJOR);
    DESC (version_minor,         GLFW_CONTEXT_VERSION_MINOR);
    INCLUDE (offscreen);
    DESC (info.on,               Show hardware info);
    DESC_H (fixlandscape.on,     Fix landscape position);
    DESC (position.x,            Window x position);
    DESC (position.y,            Window y position);
    DESC (fullscreen.on,         Window in fullscreen mode);
    DESC (fullscreen.x.on,       Window in fullscreen mode in X direction);
    DESC (fullscreen.y.on,       Window in fullscreen mode in Y direction);
  }
  int     width   = 800;
  int     height  = 800;
  struct
  {
    int x = -1, y = -1;
  } position;
  struct
  {
    bool on = true;
    int samples = 4;
  } antialiasing;
  struct
  {
    bool on = false;
  } statistics;
  string  title  = "";
  struct
  {
    bool on = false;
  } debug;
  int     version_major = 4;
  int     version_minor = 3;
  OptionsOffscreen offscreen;
  struct
  {
    bool on = false;
  } fixlandscape;
  struct
  {
    bool on = false;
  } info;
  struct
  {
    bool on = false;
    struct
    {
      bool on = false;
    } x;
    struct
    {
      bool on = false;
    } y;
  } fullscreen;
};

};


