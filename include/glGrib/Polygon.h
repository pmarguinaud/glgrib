#pragma once

#include "glGrib/Object.h"
#include "glGrib/OpenGL.h"
#include "glGrib/View.h"

namespace glGrib
{

class Polygon : public Object3D
{
public:
  Polygon () : VAID (this) {}
  void setupVertexAttributes () const;
  void render (const View &, const OptionsLight &) const override;
  int getNumberOfPoints () const
  {
    return vertexbuffer->size () / 2;
  }
  int getNumberOfLines () const
  {
    return elementbuffer->size ();
  }
  void setup (const std::vector <float> & lonlat, const std::vector <unsigned int> & ind)
  {
    vertexbuffer = glGrib::OpenGLBufferPtr<float> (lonlat);
    elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (ind);
  }
private:
  OpenGLBufferPtr<float> vertexbuffer;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  OpenGLVertexArray<Polygon> VAID;
};


}
