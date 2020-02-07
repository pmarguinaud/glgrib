#pragma once

#include "glgrib_points.h"
#include "glgrib_view.h"
#include "glgrib_options.h"
#include "glgrib_string.h"
#include <string>
#include <vector>


class glgrib_cities : public glgrib_points
{
public:
  void render (const glgrib_view &, const glgrib_options_light &) const;
  glgrib_cities & operator= (const glgrib_cities &);
  void clear ();
  void setup (const glgrib_options_cities &);
  const glgrib_options_cities & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
private:
  glgrib_options_cities opts;
  struct
  {
    glgrib_string labels;
  } d;
};

