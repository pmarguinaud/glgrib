#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"

class glgrib_program
{
public:
  GLuint programID;
  void init ();
  ~glgrib_program ();
};

#endif
