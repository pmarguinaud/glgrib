#ifndef _GLGRIB_CUBE2_H
#define _GLGRIB_CUBE2_H

#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_object.h"

class glgrib_cube2 : public glgrib_object
{
public:
  void init (float x = 0., float y = 0., float z = 0.);
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_RGB_POSITION_SCALE; }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
  ~glgrib_cube2 ();
  void def_from_vertexbuffer_col_elementbuffer (float * xyz, unsigned char * col, unsigned int * ind);
  GLuint VertexArrayID;
  GLuint colorbuffer;
  GLuint vertexbuffer, elementbuffer;
  unsigned int ncol, nt;
  int np;
  float x0, y0, z0;
};

#endif
