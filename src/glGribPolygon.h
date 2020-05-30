#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

namespace glGrib
{

class Polygon : public Object
{
public:
  Polygon () : VAID (this) {}
  void setupVertexAttributes () const;
  void render (const View &, const OptionsLight &) const override;
  virtual void clear ();
protected:
  OpenGLBufferPtr vertexbuffer, elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
  OpenGLVertexArray<Polygon> VAID;
};


}
