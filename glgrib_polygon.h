#ifndef _GLGRIB_POLYGON_H
#define _GLGRIB_POLYGON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polygon : public glgrib_object
{
public:
  void def_from_xyz_col_ind (glgrib_opengl_buffer_ptr, 
                             glgrib_opengl_buffer_ptr, 
                             glgrib_opengl_buffer_ptr);
  virtual void render (const glgrib_view *) const;
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_RGB; }
  virtual bool use_alpha () { return false; }
  virtual ~glgrib_polygon ();
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer;
  unsigned int ncol, nl;
  int numberOfPoints;
};

#endif
