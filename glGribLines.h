#pragma once

#include "glGribPolygon.h"
#include "glGribOptions.h"
#include <string>
#include <vector>

class glGribLines : public glGribPolygon
{
public:
  glGribLines & operator=(const glGribLines &);
  void setup (const glGribOptionsLines &, const std::vector<unsigned int> & = {0x00000000}, 
             const std::vector<unsigned int> & = {0x00000000});
  void render (const glGribView &, const glGribOptionsLight &) const override;
  void resize (const glGribView &) override {}
  const glGribOptionsLines & getOptions () const { return opts; }
private:
  glGribOptionsLines opts;
};

