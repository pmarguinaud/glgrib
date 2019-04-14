#ifndef _GLGRIB_POLYGON_H
#define _GLGRIB_POLYGON_H

#include "glgrib_obj.h"
#include "glgrib_opengl.h"

class polygon_t : public obj_t
{
public:
  void def_from_xyz_col_ind (const float *, const unsigned char *, const unsigned int *);
  virtual void render () const;
  ~polygon_t ();
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nl;
  int np;
};

#endif
