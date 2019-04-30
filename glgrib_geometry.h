#ifndef _GLGRIB_GEOMETRY_H
#define _GLGRIB_GEOMETRY_H

#include "glgrib_options.h"
#include "glgrib_opengl.h"

class glgrib_geometry
{
public:
  virtual 
  ~glgrib_geometry ();
  void take_xyz_ind (int *, unsigned int *, float **, unsigned int **);
  int np; 
  unsigned int nt;
  float * xyz = NULL;
  unsigned int * ind = NULL;
  GLuint vertexbuffer, elementbuffer;
};

extern glgrib_geometry * glgrib_geometry_load (const glgrib_options &);

#endif
