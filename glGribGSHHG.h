#pragma once

#include "glGribOptions.h"

#include <vector>
#include <string>

namespace glGribGSHHG
{
extern void read (const glGribOptionsLines &, int *, unsigned int *, 
		  std::vector<float> *, std::vector<unsigned int> *, 
                  const std::vector<unsigned int> & = {0x00000000}, 
                  const std::vector<unsigned int> & = {0x00000000});
};

