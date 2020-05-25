#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"

namespace glGrib
{

class Grid : public Object
{
public:
  Grid () : VAID (this) {}
  Grid & operator=(const Grid &);
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
  void clear ();
  ~Grid ();
  float getScale () const override { return d.opts.scale; }

  void setupVertexAttributes () const;

private:
  struct
  {
    String labels;
    OptionsGrid opts;
  } d;
  OpenGLVertexArray<Grid> VAID;
};


}
