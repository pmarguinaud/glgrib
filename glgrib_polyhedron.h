#ifndef _GLGRIB_POLYHEDRON_H
#define _GLGRIB_POLYHEDRON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"

class glgrib_polyhedron : public glgrib_object
{
public:
  virtual void render () const;
  ~glgrib_polyhedron ();
  void def_from_xyz_col_ind (const float *, unsigned char *, unsigned int *);
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nt;
  int np;
};

#endif
