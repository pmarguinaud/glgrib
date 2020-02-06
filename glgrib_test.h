#ifndef _GLGRIB_TEST_H
#define _GLGRIB_TEST_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_test : public glgrib_object
{
public:
  virtual void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_test () { clear (); }
  virtual void clear ();
  virtual void setup ();
  virtual void resize (const glgrib_view &) override {}
  virtual float getScale () const override { return 1; }
protected:
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};

#endif
