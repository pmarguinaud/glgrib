#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"

namespace glGrib
{

class Test : public Object3D
{
public:
  Test () : VAID (this) {}
  void render (const View &, const OptionsLight &) const override;
  void setupVertexAttributes () const;
  virtual void setup ();
  void reSize (const View &) override {}
  float getScale () const override { return 1; }
public:
  bool _visible = true;
  const bool & getVisibleRef () const override
  {
    return _visible;
  }
  OpenGLBufferPtr<float> vertexbuffer;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  OpenGLVertexArray<Test> VAID;
};


}
