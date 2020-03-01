#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"

class glGribGrid : public glGribObject
{
public:
  glGribGrid & operator=(const glGribGrid &);
  void setup (const glgrib_options_grid &);
  void render (const glGribView &, const glgrib_options_light &) const override;
  void resize (const glGribView &) override {}
  const glgrib_options_grid & getOptions () const { return opts; }
  void setColorOptions (const glgrib_option_color & o)
  {
    opts.color = o;
  }
  void setScaleOptions (float s)
  {
    opts.scale = s;
  }
  void clear ();
  ~glGribGrid ();
  float getScale () const override { return opts.scale; }
private:
  glGribString labels;
  glgrib_options_grid opts;
  GLuint VertexArrayID;
  int numberOfPoints;
};

