#pragma once

#include "glGribOptions.h"

#include <vector>
#include <string>

namespace glGribGshhg
{
extern void read (const glgrib_options_lines &, int *, unsigned int *, 
		  std::vector<float> *, std::vector<unsigned int> *, 
                  const std::vector<unsigned int> & = {0x00000000}, 
                  const std::vector<unsigned int> & = {0x00000000});
};

