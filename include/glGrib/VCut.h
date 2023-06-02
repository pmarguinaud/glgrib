#pragma once

#include "glGrib/Object.h"
#include "glGrib/OpenGL.h"
#include "glGrib/View.h"
#include "glGrib/Options.h"
#include "glGrib/Loader.h"
#include "glGrib/Geometry.h"

namespace glGrib
{

class VCut : public Object3D
{
public:
  VCut () : VAID (this) {}
  void render (const View &, const OptionsLight &) const override;
  void setupVertexAttributes () const;
  virtual void setup (Loader *, const OptionsVCut &);
  void reSize (const View &) override {}
  float getScale () const override { return 1; }
private:
  OptionsVCut opts;
  bool _visible = true;
  const bool & getVisibleRef () const override
  {
    return _visible;
  }
  OpenGLBufferPtr<float> lonlatbuffer;
  OpenGLBufferPtr<float> valuesbuffer;
  OpenGLBufferPtr<float> heightbuffer;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  OpenGLVertexArray<VCut> VAID;
  int Nx = -1, Nz = -1;
  const_GeometryPtr geometry;
  FieldMetadata meta;
};


}
