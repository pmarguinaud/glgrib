#pragma once

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_geometry.h"

class glgrib_world : public glgrib_object
{
public:
  virtual ~glgrib_world () { clear (); }
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

