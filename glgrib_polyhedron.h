#ifndef _GLGRIB_POLYHEDRON_H
#define _GLGRIB_POLYHEDRON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polyhedron : public glgrib_object
{
public:
  virtual void render (const glgrib_view *) const;
  virtual glgrib_program::kind get_program_kind () const { return glgrib_program::RGBA; }
  virtual bool use_alpha () { return true; }
  virtual ~glgrib_polyhedron () { cleanup (); }
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer;
  unsigned int numberOfColors, numberOfTriangles;
  int numberOfPoints;
  bool wireframe = false;
  void toggle_wireframe () { wireframe = ! wireframe; }
protected:
  virtual void cleanup ();
};

#endif
