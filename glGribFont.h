#pragma once

#include <vector>
#include <memory>
#include "glGribOpengl.h"
#include "glGribOptions.h"
#include "glGribProgram.h"


class glGribFont
{
public:
  void setup (const glGribOptionsFont &);
  ~glGribFont ();
  int map (char c) const
  { 
    if (c <= 32) 
      return 15; 
    int ix =     (c - 32) % 16; 
    int iy = 5 - (c - 32) / 16;
    return iy * nx + ix;
  }
  glGribProgram * getProgram () const { return glGribProgram::load (glGribProgram::FONT); }
  void select () const;
  float getAspect () const { return aspect; }
  float getPosBelow () const { return posb; }
  float getPosAbove () const { return posu; }
  const glGribOptionsFont & getOptions () const { return opts; }
private:
  glGribOptionsFont opts;
  std::vector<float> xoff, yoff;
  int nx, ny;  // Array of letters dimension
  bool ready = false;
  unsigned int texture;
  float aspect;
  float posu;  // Size above line
  float posb;  // Size below line
};

typedef std::shared_ptr<glGribFont> glgrib_font_ptr;
typedef std::shared_ptr<const glGribFont> const_glgrib_font_ptr;

glgrib_font_ptr newGlgribFontPtr (const glGribOptionsFont &);

