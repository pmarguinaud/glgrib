#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

namespace glGrib
{

class Polygon : public Object
{
public:
  void setupVertexAttributes () const;
  void render (const View &, const OptionsLight &) const override;
  virtual ~Polygon ();
  virtual void clear ();
protected:
  mutable GLuint VertexArrayID;
  OpenGLBufferPtr vertexbuffer, elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
};


}
