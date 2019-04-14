#ifndef _GLGRIB_POLYGON_H
#define _GLGRIB_POLYGON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polygon : public glgrib_object
{
public:
  void def_from_xyz_col_ind (const float *, const unsigned char *, const unsigned int *);
  virtual void render (const glgrib_view *) const;
  ~glgrib_polygon ();
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nl;
  int np;
};

#endif
