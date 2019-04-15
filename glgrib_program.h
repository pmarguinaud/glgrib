#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"


typedef enum 
{
  GLGRIB_PROGRAM_NONE=-1,
  GLGRIB_PROGRAM_RGBA=0,
  GLGRIB_PROGRAM_RGB=1,
  GLGRIB_PROGRAM_MONO=2,
  GLGRIB_PROGRAM_RGB_FLAT=3,
  GLGRIB_PROGRAM_SIZE=4,
} glgrib_program_kind;

class glgrib_program
{
public:
  glgrib_program (const char * fsc, const char * vsc) 
     : FragmentShaderCode (fsc), VertexShaderCode (vsc) { }
  ~glgrib_program ();
  void use () const;
  const char * FragmentShaderCode = NULL;
  const char * VertexShaderCode = NULL;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
  mutable bool active = false;
};

glgrib_program * glgrib_program_load (glgrib_program_kind);

#endif
