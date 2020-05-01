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
  static float defaultMin () { return std::numeric_limits<float>::max (); }
  static float defaultMax () { return std::numeric_limits<float>::min (); }

  static Palette create (const OptionsPalette &,  
                         float = defaultMin (), float = defaultMax ());

  const std::string & getName () const { return opts.name; }

  float getMin () const { return opts.min; }
  float getMax () const { return opts.max; }
  bool hasMin () const { return opts.min != defaultMin (); }
  bool hasMax () const { return opts.max != defaultMax (); }
  Palette () {}
  void setRGBA255 (GLuint) const;

  friend bool operator== (const Palette &, const Palette &);
  friend bool operator!= (const Palette &, const Palette &);

  const OptionsPalette & getOptions () const
  {
    return opts;
  }
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
  OptionColor rgba_mis;
  std::vector<OptionColor> rgba;
  void getRGBA255 (float RGBA0[256][4]) const;
  static Palette createByName (const std::string &, float, float);
  OptionsPalette opts;
};

}
