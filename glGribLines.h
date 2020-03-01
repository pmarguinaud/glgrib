#pragma once

#include "glGribPolygon.h"
#include "glGribOptions.h"
#include <string>
#include <vector>

class glGribLines : public glGribPolygon
{
public:
  glGribLines & operator=(const glGribLines &);
  void setup (const glgrib_options_lines &, const std::vector<unsigned int> & = {0x00000000}, 
             const std::vector<unsigned int> & = {0x00000000});
  void render (const glGribView &, const glgrib_options_light &) const override;
  void resize (const glGribView &) override {}
  const glgrib_options_lines & getOptions () const { return opts; }
private:
  glgrib_options_lines opts;
};

