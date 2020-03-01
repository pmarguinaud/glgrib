#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"

class glGribGrid : public glGribObject
{
public:
  glGribGrid & operator=(const glGribGrid &);
  void setup (const glGribOptionsGrid &);
  void render (const glGribView &, const glGribOptionsLight &) const override;
  void resize (const glGribView &) override {}
  const glGribOptionsGrid & getOptions () const { return opts; }
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
  glGribOptionsGrid opts;
  GLuint VertexArrayID;
  int numberOfPoints;
};

