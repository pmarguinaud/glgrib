#pragma once


#include "glGribOptions.h"

#include <vector>
#include <string>

namespace glGrib
{

namespace ShapeLib
{
extern void read (const OptionsLines &, int *, unsigned int *, 
                  std::vector<float> *, std::vector<unsigned int> *,
                  const std::string &);
};


}
