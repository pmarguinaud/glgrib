#pragma once

#include "glGribObject.h"
#include "glGribOpengl.h"
#include "glGribView.h"

class glGribTest : public glGribObject
{
public:
  void render (const glGribView &, const glgrib_options_light &) const override;
  virtual ~glGribTest () { clear (); }
  virtual void clear ();
  virtual void setup ();
  void resize (const glGribView &) override {}
  float getScale () const override { return 1; }
protected:
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};

