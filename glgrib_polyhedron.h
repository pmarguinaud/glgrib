#ifndef _GLGRIB_POLYHEDRON_H
#define _GLGRIB_POLYHEDRON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polyhedron : public glgrib_object
{
public:
  virtual void render (const glgrib_view *) const;
  virtual int get_program_kind () { return 0; }
  virtual bool use_alpha () { return true; }
  ~glgrib_polyhedron ();
  void def_from_xyz_col_ind (const float *, unsigned char *, unsigned int *);
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  unsigned int ncol, nt;
  int np;
};

#endif
