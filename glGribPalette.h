#pragma once

#include "glGribOpengl.h"
#include "glGribOptions.h"
#include "glGribFieldMetadata.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <limits>

class glGribPalette
{
public:
  static const float defaultMin;
  static const float defaultMax;

  static glGribPalette next (const glGribPalette &, float = defaultMin, float = defaultMax);

  static glGribPalette create (const glgrib_options_palette &,  
                                float = defaultMin, float = defaultMax);

  const std::string & getName () { return opts.name; }

  glgrib_option_color rgba_mis;
  std::vector<glgrib_option_color> rgba;
  float getMin () const { return opts.min; }
  float getMax () const { return opts.max; }
  bool hasMin () const { return opts.min != defaultMin; }
  bool hasMax () const { return opts.max != defaultMax; }
  glGribPalette (std::ifstream &);
  glGribPalette () {}
  glGribPalette (const std::string & n, bool) 
  {
    opts.name = n;
  }
  template <typename... Types> 
  glGribPalette (const std::string & n, int r, int g, int b, int a,Types... vars)
  {
    glGribPalette p = glGribPalette (n, true, vars...);
    p.rgba_mis = glgrib_option_color (r, g, b, a);
    *this = p;
    register_ (p);
  }
  template <typename... Types> 
  glGribPalette (float min_, float max_, const std::string & n, 
		  int r, int g, int b, int a, Types... vars)
  {
    glGribPalette p = glGribPalette (n, true, vars...);
    p.rgba_mis = glgrib_option_color (r, g, b, a);
    p.opts.min = min_;
    p.opts.max = max_;
    *this = p;
    register_ (p);
  }
  friend std::ostream & operator << (std::ostream &, const glGribPalette &);
  void setRGBA255 (GLuint) const;
  glGribPalette & register_ (const glGribPalette &);
  friend bool operator== (const glGribPalette &, const glGribPalette &);
  friend bool operator!= (const glGribPalette &, const glGribPalette &);

  const glgrib_options_palette & getOptions () const
  {
    return opts;
  }
  void setMin (float min) { opts.min = min; }
  void setMax (float max) { opts.max = max; }

  const std::vector<float> & getValues () const { return opts.values; }

  glgrib_option_color getColor (float) const;
  int getColorIndex (float) const;

  bool isLinear () const
  {
    return opts.linear.on && (opts.values.size () == opts.colors.size () + 1);
  }

  float getScale () const
  {
    return opts.scale;
  }

  float getOffset () const
  {
    return opts.offset;
  }


private:
  void getRGBA255 (float RGBA0[256][4]) const;
  static glGribPalette & create_by_name (const std::string &);
  glgrib_options_palette opts;
  template <typename T, typename... Types> 
  glGribPalette (const std::string & n, bool record, T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (glgrib_option_color (r, g, b, a));
    glGribPalette p = glGribPalette (n, false, vars...);
    for (std::vector<glgrib_option_color>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
    opts.name = n;
  }
};

extern glGribPalette palette_cold_hot;
extern glGribPalette palette_cloud;
extern glGribPalette palette_cloud_auto;




