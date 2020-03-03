#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

class glGribPolygon : public glGribObject
{
public:
  void setupVertexAttributes ();
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribPolygon ();
  virtual void clear ();
protected:
  GLuint VertexArrayID;
  glGribOpenGLBufferPtr vertexbuffer, elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
};

