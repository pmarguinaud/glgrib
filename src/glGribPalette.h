#pragma once

#include <glm/glm.hpp>

#include "glGribOpenGL.h"
#include "glGribProgram.h"
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

  explicit Palette (const OptionsPalette &,  
                    const float = defaultMin (), const float = defaultMax ());

  const std::string & getName () const { return opts.name; }

  float getMin () const { return opts.min; }
  float getMax () const { return opts.max; }
  bool hasMin () const { return opts.min != defaultMin (); }
  bool hasMax () const { return opts.max != defaultMax (); }
  bool fixed () const { return opts.fixed.on; }

  Palette () {}
  void set (glGrib::Program *) const;

  friend bool operator== (const Palette &, const Palette &);
  friend bool operator!= (const Palette &, const Palette &);

  bool isEqual (const Palette & p) const;

  const OptionsPalette & getOptions () const
  {
    return opts;
  }
  const std::vector<float> & getValues () const { return opts.values; }

  OptionColor getColor (const float) const;
  int getColorIndex (const float) const;

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

  size_t size () const
  {
    return rgba_.size ();
  }


private:
  OptionColor rgba_mis;
  std::vector<OptionColor> rgba;
  std::vector<glm::vec4> rgba_;
  void setMinMax (const float min, const float max) 
  { 
    if (opts.min == defaultMin ()) 
      opts.min = min; 
    if (opts.max == defaultMax ()) 
      opts.max = max; 
  }
  void computergba_255 ();
  void createByName (const std::string &, const float, const float);
  void createByOpts (const glGrib::OptionsPalette &, float, float);
  void createValueLinearRange (const float, const float, const int);
  void createRainbow ();
  void createGradient ();
  void createDiscrete ();
  OptionsPalette opts;
  OpenGLBufferPtr<glm::vec4> rgba_buffer;
};

}
