#pragma once

#include "glGribPolygon.h"
#include "glGribOptions.h"
#include <string>
#include <vector>

namespace glGrib
{

class Lines : public Polygon
{
public:
  void setup (const OptionsLines &, const std::vector<unsigned int> & = {0x00000000}, 
             const std::vector<unsigned int> & = {0x00000000});
  void render (const View &, const OptionsLight &) const override;
  void reSize (const View &) override {}
  const OptionsLines & getOptions () const { return opts; }
private:
  OptionsLines opts;
};


}
