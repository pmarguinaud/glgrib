#ifndef _GLGRIB_POLYHEDRON_H
#define _GLGRIB_POLYHEDRON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polyhedron : public glgrib_object
{
public:
  virtual void render (const glgrib_view *) const;
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_RGBA; }
  virtual bool use_alpha () { return true; }
  virtual ~glgrib_polyhedron () { cleanup (); }
  void def_from_xyz_col_ind (const float *, unsigned char *, unsigned int *);
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer;
  unsigned int ncol, nt;
  int np;
  bool wireframe = false;
  void toggle_wireframe () { wireframe = ! wireframe; }
protected:
  void cleanup ();
};

#endif
