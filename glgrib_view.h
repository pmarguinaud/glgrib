#ifndef _GLGRIB_VIEW_H
#define _GLGRIB_VIEW_H

#include "glgrib_opengl.h"
#include "glgrib_program.h"

class view_t
{
public:
  float rc = 6.0, latc = 0., lonc = 0., fov = 20.;
  GLuint MatrixID;
  void init (program_t *);
};

#endif
