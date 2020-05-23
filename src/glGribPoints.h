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

  Points () : d (this) { }
  ~Points ();
  
  Points & operator= (const Points &);
  void setupVertexAttributes () const;
  virtual void clear ();

  void setup (const OptionsPoints &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const View &, const OptionsLight &) const override;
  void render (const View &, const OptionsLight &, const int, const int) const;
  const OptionsPoints & getOptions () const { return d.opts; }
  void reSize (const View &) override {}
  
  const glGrib::Palette & getPalette () const
  {
    return d.p;
  }

private:
  class _d 
  {
  public:
    _d (Points * p) : VAID (p) {}
    float min, max;
    OptionsPoints opts;
    int len;
    OpenGLBufferPtr llsbuffer;
    glGrib::Palette p;
    mutable OpenGLVertexArray<Points> VAID;
  };
  _d d;
};


}
