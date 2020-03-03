#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

namespace glGrib
{

class Test : public Object
{
public:
  void render (const View &, const OptionsLight &) const override;
  virtual ~Test () { clear (); }
  virtual void clear ();
  virtual void setup ();
  void resize (const View &) override {}
  float getScale () const override { return 1; }
protected:
  OpenGLBufferPtr vertexbuffer, elementbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};


}
