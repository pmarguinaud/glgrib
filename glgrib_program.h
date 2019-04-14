#ifndef _GLGRIB_PROGRAM_H
#define _GLGRIB_PROGRAM_H

#include "glgrib_opengl.h"

class program_t
{
public:
  GLuint programID;
  void init ();
  ~program_t ();
};

#endif
