#pragma once

#include "glGribPoints.h"
#include "glGribView.h"
#include "glGribOptions.h"
#include "glGribString.h"
#include <string>
#include <vector>


namespace glGrib
{

class GeoPoints : public Points
{
public:
  void render (const View &, const OptionsLight &) const;
  void clear ();
  void setup (const OptionsGeoPoints &);
  void update ();
  const OptionsGeoPoints & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
private:
  OptionsGeoPoints opts;
  int time = 0;
};


}
