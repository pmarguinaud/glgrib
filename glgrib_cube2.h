#ifndef _GLGRIB_CUBE2_H
#define _GLGRIB_CUBE2_H

#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_coords.h"

class glgrib_coords_cube : public glgrib_coords
{
public:
  void init (const glgrib_view * view = NULL);
};

class glgrib_cube2 : public glgrib_object
{
public:
  void init (const glgrib_coords_cube * coords, float x = 0., float y = 0., float z = 0.);
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_RGB_POSITION_SCALE; }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
  ~glgrib_cube2 ();
  void def_from_vertexbuffer_col_elementbuffer (const glgrib_coords_cube *, unsigned char * col);
  GLuint VertexArrayID;
  GLuint colorbuffer;
  unsigned int ncol, nt;
  int np;
  float x0, y0, z0;
};

#endif
