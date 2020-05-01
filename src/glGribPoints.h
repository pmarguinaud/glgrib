#pragma once

#include "glGribView.h"
#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include "glGribPalette.h"
#include "glGribView.h"
#include "glGribObject.h"
#include "glGribPalette.h"
#include <string>
#include <vector>


namespace glGrib
{

class Points : public Object
{
public:
  ~Points ();
  
  Points & operator= (const Points &);
  void setupVertexAttributes ();
  virtual void clear ();

  void setup (const OptionsPoints &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const View &, const OptionsLight &) const override;
  const OptionsPoints & getOptions () const { return d.opts; }
  void reSize (const View &) override {}
private:
  struct
  {
    float min, max;
    OptionsPoints opts;
    int len;
    OpenGLBufferPtr llsbuffer;
    glGrib::Palette p;
  } d;
  GLuint VertexArrayID = 0;
};


}
