#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribGeometry.h"

class glGribWorld : public glGribObject
{
public:
  virtual ~glGribWorld () { clear (); }
  const_glgrib_geometry_ptr getGeometry () const { return geometry; }
  virtual void clear ();
protected:
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer, heightbuffer, mpivbuffer;
  const_glgrib_geometry_ptr geometry;
  int numberOfPoints;
  unsigned int numberOfTriangles;
  GLuint VertexArrayID = 0;
  GLuint VertexArrayID_frame = 0;
};

