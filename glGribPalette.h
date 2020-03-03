#pragma once

#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include "glGribFieldMetadata.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <limits>

namespace glGrib
{

class Palette
{
public:
  static const float defaultMin;
  static const float defaultMax;

  static Palette next (const Palette &, float = defaultMin, float = defaultMax);

  static Palette create (const OptionsPalette &,  
                                float = defaultMin, float = defaultMax);

  const std::string & getName () { return opts.name; }

  OptionColor rgba_mis;
  std::vector<OptionColor> rgba;
  float getMin () const { return opts.min; }
  float getMax () const { return opts.max; }
  bool hasMin () const { return opts.min != defaultMin; }
  bool hasMax () const { return opts.max != defaultMax; }
  Palette (std::ifstream &);
  Palette () {}
  Palette (const std::string & n, bool) 
  {
    opts.name = n;
  }
  template <typename... Types> 
  Palette (const std::string & n, int r, int g, int b, int a,Types... vars)
  {
    Palette p = Palette (n, true, vars...);
    p.rgba_mis = OptionColor (r, g, b, a);
    *this = p;
    register_ (p);
  }
  template <typename... Types> 
  Palette (float min_, float max_, const std::string & n, 
		  int r, int g, int b, int a, Types... vars)
  {
    Palette p = Palette (n, true, vars...);
    p.rgba_mis = OptionColor (r, g, b, a);
    p.opts.min = min_;
    p.opts.max = max_;
    *this = p;
    register_ (p);
  }
  friend std::ostream & operator << (std::ostream &, const Palette &);
  void setRGBA255 (GLuint) const;
  Palette & register_ (const Palette &);
  friend bool operator== (const Palette &, const Palette &);
  friend bool operator!= (const Palette &, const Palette &);

  const OptionsPalette & getOptions () const
  {
    return opts;
  }
  void setMin (float min) { opts.min = min; }
  void setMax (float max) { opts.max = max; }

  const std::vector<float> & getValues () const { return opts.values; }

  OptionColor getColor (float) const;
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
  static Palette & createByName (const std::string &);
  OptionsPalette opts;
  template <typename T, typename... Types> 
  Palette (const std::string & n, bool record, T r, T g, T b, T a, Types... vars)
  {
    rgba.push_back (OptionColor (r, g, b, a));
    Palette p = Palette (n, false, vars...);
    for (std::vector<OptionColor>::iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
      rgba.push_back (*it);
    opts.name = n;
  }
};

extern Palette paletteColdHot;
extern Palette paletteCloud;
extern Palette paletteCloudAuto;





}
