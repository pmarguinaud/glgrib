#pragma once


#include "glGribOptions.h"

#include <vector>
#include <string>

namespace glGribShapeLib
{
extern void read (const glGribOptionsLines &, int *, unsigned int *, 
                  std::vector<float> *, std::vector<unsigned int> *,
                  const std::string &);
};

