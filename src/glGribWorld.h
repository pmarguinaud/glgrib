#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribGeometry.h"

namespace glGrib
{

class World : public Object
{
public:
  virtual ~World () { clear (); }
  const_GeometryPtr getGeometry () const { return geometry; }
  virtual void clear ();
protected:
  OpenGLBufferPtr vertexbuffer, colorbuffer, elementbuffer, heightbuffer, mpivbuffer;
  const_GeometryPtr geometry;
  mutable GLuint VertexArrayID = 0;
//glGrib::OpenGLVertexArray VertexArray;
  mutable GLuint VertexArrayID_frame = 0;
};


}
