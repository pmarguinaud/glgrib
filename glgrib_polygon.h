#pragma once

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"

class glgrib_polygon : public glgrib_object
{
public:
  void setupVertexAttributes ();
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_polygon ();
  virtual void clear ();
protected:
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
};

