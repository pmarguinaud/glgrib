#ifndef _GLGRIB_PALETTE_H
#define _GLGRIB_PALETTE_H

#include "glgrib_opengl.h"

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <limits>

typedef unsigned char byte;

class glgrib_rgba
{
public:
  glgrib_rgba () {}
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
  glgrib_rgba rgba_mis;
  std::vector<glgrib_rgba> rgba;
  std::string name;
  float min = std::numeric_limits<float>::max(), max = std::numeric_limits<float>::min();
  float getMin () const { return min; }
  float getMax () const { return max; }
  bool hasMin () const { return min != std::numeric_limits<float>::max(); }
  bool hasMax () const { return max != std::numeric_limits<float>::min(); }
  glgrib_palette () {}
  glgrib_palette (const std::string & n, bool) : name (n)
  {
  }
  template <typename... Types> 
  glgrib_palette (const std::string & n, int r, int g, int b, int a,Types... vars)
  {
    glgrib_palette p = glgrib_palette (n, true, vars...);
    *this = p;
    rgba_mis = glgrib_rgba (r, g, b, a);
    register_ (p);
  }
  template <typename... Types> 
  glgrib_palette (float min_, float max_, const std::string & n, 
		  int r, int g, int b, int a, Types... vars)
  {
    glgrib_palette p = glgrib_palette (n, true, vars...);
    *this = p;
    rgba_mis = glgrib_rgba (r, g, b, a);
    register_ (p);
    min = min_;
    max = max_;
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
extern glgrib_palette palette_cloud_auto;

glgrib_palette & get_palette_by_name (const std::string &);



#endif
