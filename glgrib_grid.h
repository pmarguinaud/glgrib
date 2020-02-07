#pragma once

#include "glgrib_object.h"
#include "glgrib_options.h"
#include "glgrib_string.h"

class glgrib_grid : public glgrib_object
{
public:
  glgrib_grid & operator=(const glgrib_grid &);
  void setup (const glgrib_options_grid &);
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  void resize (const glgrib_view &) override {}
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
  ~glgrib_grid ();
  float getScale () const override { return opts.scale; }
private:
  glgrib_string labels;
  glgrib_options_grid opts;
  GLuint VertexArrayID;
  int numberOfPoints;
};

