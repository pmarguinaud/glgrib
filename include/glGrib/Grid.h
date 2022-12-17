#pragma once

#include "glGrib/Object.h"
#include "glGrib/Options.h"
#include "glGrib/String.h"

namespace glGrib
{

class Grid : public Object3D
{
public:
  Grid () : VAID (this) {}
  void setup (const OptionsGrid &);
  void render (const View &, const OptionsLight &) const override;
  void reSize (const View &) override {}
  const OptionsGrid & getOptions () const { return d.opts; }
  
  void setColorOptions (const OptionColor & o)
  {
    d.opts.color = o;
  }
  void setScaleOptions (float s)
  {
    d.opts.scale = s;
  }
  float getScale () const override { return d.opts.scale; }

  void setupVertexAttributes () const 
  {
  }

private:
  const bool & getVisibleRef () const override
  {
    return d.opts.visible.on;
  }
  struct
  {
    String3D<1,0> labels;
    OptionsGrid opts;
  } d;
  OpenGLVertexArray<Grid> VAID;
};


}
