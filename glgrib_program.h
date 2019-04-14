#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"

class glgrib_program
{
public:
  glgrib_program (const char * fsc, const char * vsc) : FragmentShaderCode (fsc), VertexShaderCode (vsc) { }
  ~glgrib_program ();
  void use ();
  const char * FragmentShaderCode = NULL;
  const char * VertexShaderCode = NULL;
  GLuint programID;
  GLuint matrixID;
  bool loaded = false;
};

glgrib_program * glgrib_program_load (int);

#endif
