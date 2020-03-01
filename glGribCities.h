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
  void render (const glGribView &, const glGribOptionsLight &) const;
  glGribCities & operator= (const glGribCities &);
  void clear ();
  void setup (const glGribOptionsCities &);
  const glGribOptionsCities & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
private:
  glGribOptionsCities opts;
  struct
  {
    glGribString labels;
  } d;
};

