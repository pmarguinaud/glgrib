#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribGeometry.h"

class glGribWorld : public glGribObject
{
public:
  virtual ~glGribWorld () { clear (); }
  const_glGribGeometryPtr getGeometry () const { return geometry; }
  virtual void clear ();
protected:
  glGribOpenGLBufferPtr vertexbuffer, colorbuffer, elementbuffer, heightbuffer, mpivbuffer;
  const_glGribGeometryPtr geometry;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
  GLuint VertexArrayID_frame = 0;
};

