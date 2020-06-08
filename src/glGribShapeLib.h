#pragma once


#include "glGribOptionsLines.h"

#include <vector>
#include <string>

namespace glGrib
{

namespace ShapeLib
{
extern void read (const OptionsLines &, 
                  std::vector<float> *, std::vector<unsigned int> *,
                  const std::string &);
};


}
