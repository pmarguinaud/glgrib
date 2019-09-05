#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_geometry.h"

class glgrib_world : public glgrib_object
{
public:
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual bool use_alpha () { return true; }
  virtual ~glgrib_world () { clear (); }
  const_glgrib_geometry_ptr getGeometry () const { return geometry; }
  virtual void clear ();
protected:
  glgrib_opengl_buffer_ptr vertexbuffer, colorbuffer, elementbuffer;
  const_glgrib_geometry_ptr geometry;
  int numberOfPoints;
  unsigned int numberOfColors, numberOfTriangles;
  GLuint VertexArrayID = 0;
};

#endif
