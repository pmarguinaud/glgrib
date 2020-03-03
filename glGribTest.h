#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

class glGribTest : public glGribObject
{
public:
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribTest () { clear (); }
  virtual void clear ();
  virtual void setup ();
  void resize (const glGribView &) override {}
  float getScale () const override { return 1; }
protected:
  glGribOpenGLBufferPtr vertexbuffer, elementbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
};

