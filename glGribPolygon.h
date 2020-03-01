#pragma once

#include "glGribObject.h"
#include "glGribOpengl.h"
#include "glGribView.h"

class glGribPolygon : public glGribObject
{
public:
  void setupVertexAttributes ();
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribPolygon ();
  virtual void clear ();
protected:
  GLuint VertexArrayID;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  unsigned int numberOfLines;
  int numberOfPoints;
};

