#pragma once

#include <vector>
#include <memory>
#include "glGrib/OpenGL.h"
#include "glGrib/Options.h"
#include "glGrib/Program.h"


namespace glGrib
{

class Font
{
public:
  static void clearCache ();
  void setup (const OptionsFont &);
  int map (char c) const
  { 
    if (c <= 32) 
      return 15; 
    int ix =     (c - 32) % 16; 
    int iy = 5 - (c - 32) / 16;
    return iy * nx + ix;
  }
  Program * getProgram () const { return Program::load ("FONT"); }
  void select () const;
  float getAspect () const { return aspect; }
  float getPosBelow () const { return posb; }
  float getPosAbove () const { return posu; }
  const OptionsFont & getOptions () const { return opts; }
private:
  OptionsFont opts;
  std::vector<float> xoff, yoff;
  int nx, ny;  // Array of letters dimension
  bool ready = false;
  OpenGLTexturePtr texture;
  float aspect;
  float posu;  // Size above line
  float posb;  // Size below line
};

typedef std::shared_ptr<Font> FontPtr;
typedef std::shared_ptr<const Font> const_FontPtr;

FontPtr getGlGribFontPtr (const OptionsFont &);

}
