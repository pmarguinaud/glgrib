#ifndef _GLGRIB_FONT_H
#define _GLGRIB_FONT_H

#include <vector>
#include <memory>
#include "glgrib_opengl.h"
#include "glgrib_options.h"


class glgrib_font
{
public:
  void init (const glgrib_options_font &);
  ~glgrib_font ();
  int map (char c) const
  { 
    if (c <= 32) 
      return 15; 
    int ix =     (c - 32) % 16; 
    int iy = 5 - (c - 32) / 16;
    return iy * nx + ix;
  }
  void loadShader ();
  GLuint getProgram () const { return programID; }
  void select () const;
  float getAspect () const { return aspect; }
  float getPosBelow () const { return posb; }
  float getPosAbove () const { return posu; }
private:
  glgrib_options_font opts;
  static bool programReady;
  static GLuint programID;
  std::vector<float> xoff, yoff;
  int nx, ny;  // Array of letters dimension
  bool ready = false;
  unsigned int texture;
  float aspect;
  float posu;  // Size above line
  float posb;  // Size below line
};

typedef std::shared_ptr<glgrib_font> glgrib_font_ptr;
typedef std::shared_ptr<const glgrib_font> const_glgrib_font_ptr;

glgrib_font_ptr new_glgrib_font_ptr (const glgrib_options_font &);

#endif
