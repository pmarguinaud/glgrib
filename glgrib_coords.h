#ifndef _GLGRIB_COORDS_H
#define _GLGRIB_COORDS_H

#include "glgrib_opengl.h"

class glgrib_coords
{
public:
  ~glgrib_coords ();
  GLuint vertexbuffer, elementbuffer;
  int np, nt;
};

#endif
