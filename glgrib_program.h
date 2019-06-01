#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"


class glgrib_program
{
public:
  typedef enum 
  {
    NONE=-1,
    RGBA=0,
    RGB=1,
    MONO=2,
    RGB_FLAT=3,
    RGB_POSITION_SCALE=4,
    GRADIENT_FLAT_SCALE_SCALAR=5,
    FLAT_TEX=6,
    GRADIENT_FLAT_SCALE_VECTOR=7,
    SIZE=8,
  } kind;

  glgrib_program (const std::string & fsc, const std::string & vsc) 
     : FragmentShaderCode (fsc), VertexShaderCode (vsc) { }
  virtual ~glgrib_program ();
  void use () const;
  std::string FragmentShaderCode;
  std::string VertexShaderCode;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
  mutable bool active = false;
};

glgrib_program * glgrib_program_load (glgrib_program::kind);

#endif
