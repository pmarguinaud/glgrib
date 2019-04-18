#ifndef _GLGRIB_PALETTE_H
#define _GLGRIB_PALETTE_H

#include "glgrib_opengl.h"

#include <vector>
#include <iostream>

typedef unsigned char byte;

class glgrib_rgba
{
public:
  glgrib_rgba (byte _r, byte _g, byte _b, byte _a) 
   : r (_r), g (_g), b (_b), a (_a) {}
  byte r = 255, g = 255, b = 255, a = 255;
  friend std::ostream & operator << (std::ostream &out, const glgrib_rgba & p)
  {
    out << "{" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b << ", " << (int)p.a << "}";
  }
};

class glgrib_palette
{
public:
  std::vector<glgrib_rgba> rgba;
  glgrib_palette ()
  {
  }
  template <typename T, typename... Types> 
  glgrib_palette (T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (glgrib_rgba (r, g, b, a));
    glgrib_palette p = glgrib_palette (vars...);
    for (std::vector<glgrib_rgba>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
  }
  friend std::ostream & operator << (std::ostream &, const glgrib_palette &);
  void setRGBA255 (GLuint);
};





#endif
