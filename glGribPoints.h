#pragma once

#include "glGribView.h"
#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include "glGribPalette.h"
#include "glGribView.h"
#include "glGribObject.h"
#include <string>
#include <vector>


class glGribPoints : public glGribObject
{
public:
  ~glGribPoints ();
  
  glGribPoints & operator= (const glGribPoints &);
  void setupVertexAttributes ();
  virtual void clear ();

  void setup (const glGribOptionsPoints &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const glGribView &, const glGribOptionsLight &) const override;
  const glGribOptionsPoints & getOptions () const { return d.opts; }
  void resize (const glGribView &) override {}
private:
  struct
  {
    float min, max;
    glGribOptionsPoints opts;
    int len;
    glGribOpenGLBufferPtr llsbuffer;
  } d;
  GLuint VertexArrayID = 0;
};

