#pragma once

#include "glGrib/Points.h"
#include "glGrib/View.h"
#include "glGrib/Options.h"
#include "glGrib/String.h"
#include <string>
#include <vector>


namespace glGrib
{

class GeoPoints : public Points
{
public:
  void render (const View &, const OptionsLight &) const;
  void setup (const OptionsGeoPoints &);
  void update ();
  const OptionsGeoPoints & getOptions () const { return opts; }
  float getScale () const override { return opts.points.scale; }
  bool useColorBar () const { return true; }
  GeoPoints * clone () const
  {
    GeoPoints * geopoint = new GeoPoints ();
    *geopoint = *this;
    return geopoint;
  }
private:
  OptionsGeoPoints opts;
  int time = 0;
};


}
