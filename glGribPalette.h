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

  static glGribPalette create (const glGribOptionsPalette &,  
                                float = defaultMin, float = defaultMax);

  const std::string & getName () { return opts.name; }

  glGribOptionColor rgba_mis;
  std::vector<glGribOptionColor> rgba;
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
    p.rgba_mis = glGribOptionColor (r, g, b, a);
    *this = p;
    register_ (p);
  }
  template <typename... Types> 
  glGribPalette (float min_, float max_, const std::string & n, 
		  int r, int g, int b, int a, Types... vars)
  {
    glGribPalette p = glGribPalette (n, true, vars...);
    p.rgba_mis = glGribOptionColor (r, g, b, a);
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

  const glGribOptionsPalette & getOptions () const
  {
    return opts;
  }
  void setMin (float min) { opts.min = min; }
  void setMax (float max) { opts.max = max; }

  const std::vector<float> & getValues () const { return opts.values; }

  glGribOptionColor getColor (float) const;
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
  static glGribPalette & createByName (const std::string &);
  glGribOptionsPalette opts;
  template <typename T, typename... Types> 
  glGribPalette (const std::string & n, bool record, T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (glGribOptionColor (r, g, b, a));
    glGribPalette p = glGribPalette (n, false, vars...);
    for (std::vector<glGribOptionColor>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
    opts.name = n;
  }
};

extern glGribPalette paletteColdHot;
extern glGribPalette paletteCloud;
extern glGribPalette paletteCloudAuto;




