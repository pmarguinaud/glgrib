#ifndef _GLGRIB_PROG_H
#define _GLGRIB_PROG_H

#include "glgrib_opengl.h"

class prog_t
{
public:
  GLuint programID;
  void init ();
  ~prog_t ();
};

#endif
