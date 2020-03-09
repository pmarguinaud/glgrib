#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"

namespace glGrib
{

class Grid : public Object
{
public:
  Grid & operator=(const Grid &);
  void setup (const OptionsGrid &);
  void render (const View &, const OptionsLight &) const override;
  void reSize (const View &) override {}
  const OptionsGrid & getOptions () const { return opts; }
  void setColorOptions (const OptionColor & o)
  {
    opts.color = o;
  }
  void setScaleOptions (float s)
  {
    opts.scale = s;
  }
  void clear ();
  ~Grid ();
  float getScale () const override { return opts.scale; }
private:
  String labels;
  OptionsGrid opts;
  GLuint VertexArrayID;
  int numberOfPoints;
};


}
