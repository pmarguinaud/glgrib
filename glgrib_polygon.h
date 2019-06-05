#ifndef _GLGRIB_POLYGON_H
#define _GLGRIB_POLYGON_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polygon : public glgrib_object
{
public:
  void setupVertexAttributes ();
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual bool use_alpha () { return false; }
  virtual ~glgrib_polygon ();
protected:
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  unsigned int numberOfColors, numberOfLines;
  int numberOfPoints;
  virtual void cleanup ();
};

#endif
