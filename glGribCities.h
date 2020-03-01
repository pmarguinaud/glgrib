#pragma once

#include "glGribPoints.h"
#include "glGribView.h"
#include "glGribOptions.h"
#include "glGribString.h"
#include <string>
#include <vector>


class glGribCities : public glGribPoints
{
public:
  void render (const glGribView &, const glgrib_options_light &) const;
  glGribCities & operator= (const glGribCities &);
  void clear ();
  void setup (const glgrib_options_cities &);
  const glgrib_options_cities & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
private:
  glgrib_options_cities opts;
  struct
  {
    glGribString labels;
  } d;
};

