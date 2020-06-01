#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

namespace glGrib
{

class Polygon : public Object3D
{
public:
  Polygon () : VAID (this) {}
  void setupVertexAttributes () const;
  void render (const View &, const OptionsLight &) const override;
protected:
  OpenGLBufferPtr<float> vertexbuffer;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
  OpenGLVertexArray<Polygon> VAID;
};


}
