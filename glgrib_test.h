#ifndef _GLGRIB_TEST_H
#define _GLGRIB_TEST_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_geometry.h"

class glgrib_test : public glgrib_object
{
public:
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_test () { clear (); }
  const_glgrib_geometry_ptr getGeometry () const { return geometry; }
  virtual void clear ();
  virtual void setup ();
  virtual void resize (const glgrib_view &) {}
protected:
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  const_glgrib_geometry_ptr geometry;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};

#endif
