#ifndef _GLGRIB_PALETTE_H
#define _GLGRIB_PALETTE_H

#include "glgrib_opengl.h"
#include "glgrib_field_metadata.h"
#include "glgrib_options.h"

#include <vector>
#include <iostream>
#include <fstream>
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
  friend bool operator== (const glgrib_rgba & rgba1, const glgrib_rgba & rgba2)
  {
    return (rgba1.r == rgba2.r) && (rgba1.g == rgba2.g) && (rgba1.b == rgba2.b) && (rgba1.a == rgba2.a);
  }
  friend bool operator!= (const glgrib_rgba & rgba1, const glgrib_rgba & rgba2)
  {
    return ! (rgba1 == rgba2);
  }
};

class glgrib_palette
{
public:
  static const float defaultMin;
  static const float defaultMax;

  static glgrib_palette next (const glgrib_palette &, float = defaultMin, float = defaultMax);

  static glgrib_palette create (const glgrib_options_palette &,  
                                float = defaultMin, float = defaultMax,
                                const glgrib_field_metadata & = glgrib_field_metadata ());

  const std::string & getName () { return opts.name; }

  void save (const glgrib_field_metadata &) const;
  glgrib_rgba rgba_mis;
  std::vector<glgrib_rgba> rgba;
  float getMin () const { return opts.min; }
  float getMax () const { return opts.max; }
  bool hasMin () const { return opts.min != defaultMin; }
  bool hasMax () const { return opts.max != defaultMax; }
  glgrib_palette (std::ifstream &);
  glgrib_palette () {}
  glgrib_palette (const std::string & n, bool) 
  {
    opts.name = n;
  }
  template <typename... Types> 
  glgrib_palette (const std::string & n, int r, int g, int b, int a,Types... vars)
  {
    glgrib_palette p = glgrib_palette (n, true, vars...);
    p.rgba_mis = glgrib_rgba (r, g, b, a);
    *this = p;
    register_ (p);
  }
  template <typename... Types> 
  glgrib_palette (float min_, float max_, const std::string & n, 
		  int r, int g, int b, int a, Types... vars)
  {
    glgrib_palette p = glgrib_palette (n, true, vars...);
    p.rgba_mis = glgrib_rgba (r, g, b, a);
    p.opts.min = min_;
    p.opts.max = max_;
    *this = p;
    register_ (p);
  }
  friend std::ostream & operator << (std::ostream &, const glgrib_palette &);
  void setRGBA255 (GLuint) const;
  glgrib_palette & register_ (const glgrib_palette &);
  friend bool operator== (const glgrib_palette &, const glgrib_palette &);
  friend bool operator!= (const glgrib_palette &, const glgrib_palette &);

  const glgrib_options_palette & getOptions () const
  {
    return opts;
  }
  void setMin (float min) { opts.min = min; }
  void setMax (float max) { opts.max = max; }

private:
  static glgrib_palette create_by_meta (const glgrib_field_metadata &);
  static glgrib_palette & create_by_name (const std::string &);
  glgrib_options_palette opts;
  template <typename T, typename... Types> 
  glgrib_palette (const std::string & n, bool record, T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (glgrib_rgba (r, g, b, a));
    glgrib_palette p = glgrib_palette (n, false, vars...);
    for (std::vector<glgrib_rgba>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
    opts.name = n;
  }
};

extern glgrib_palette palette_cold_hot;
extern glgrib_palette palette_cloud;
extern glgrib_palette palette_cloud_auto;




#endif
