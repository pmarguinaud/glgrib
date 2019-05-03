#ifndef _GLGRIB_PALETTE_H
#define _GLGRIB_PALETTE_H

#include "glgrib_opengl.h"

#include <vector>
#include <iostream>
#include <string>
#include <map>

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
  std::string name;
  glgrib_palette () {}
  glgrib_palette (const std::string & n, bool) : name (n)
  {
  }
  template <typename... Types> 
  glgrib_palette (const std::string & n, Types... vars)
  {
    glgrib_palette p = glgrib_palette (n, true, vars...);
    *this = p;
    register_ (p);
  }
  friend std::ostream & operator << (std::ostream &, const glgrib_palette &);
  void setRGBA255 (GLuint) const;
private:
  template <typename T, typename... Types> 
  glgrib_palette (const std::string & n, bool record, T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (glgrib_rgba (r, g, b, a));
    glgrib_palette p = glgrib_palette (n, false, vars...);
    for (std::vector<glgrib_rgba>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
    name = n;
  }
  void register_ (const glgrib_palette &);
};


extern glgrib_palette palette_cold_hot;
extern glgrib_palette palette_cloud;

glgrib_palette & get_palette_by_name (const std::string &);



#endif
