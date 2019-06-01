#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_geometry.h"

class glgrib_world : public glgrib_object
{
public:
  virtual void render (const glgrib_view *) const;
  virtual glgrib_program::kind get_program_kind () const { return glgrib_program::RGBA; }
  virtual bool use_alpha () { return true; }
  virtual ~glgrib_world () { cleanup (); }
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer;
  void toggle_wireframe () { wireframe = ! wireframe; }
  const_glgrib_geometry_ptr geometry;
protected:
  int numberOfPoints;
  virtual void cleanup ();
  bool wireframe = false;
  unsigned int numberOfColors, numberOfTriangles;
  GLuint VertexArrayID;
};

#endif
