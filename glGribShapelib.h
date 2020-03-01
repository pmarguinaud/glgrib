#pragma once


#include "glGribOptions.h"

#include <vector>
#include <string>

namespace glGribShapelib
{
extern void read (const glgrib_options_lines &, int *, unsigned int *, 
                  std::vector<float> *, std::vector<unsigned int> *,
                  const std::string &);
};

