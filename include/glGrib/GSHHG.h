#pragma once

#include "glGrib/Options.h"

#include <vector>
#include <string>

namespace glGrib
{

namespace GSHHG
{
void read (const OptionsLines &, 
 	   std::vector<float> *, std::vector<unsigned int> *, 
           const std::vector<unsigned int> & = {0x00000000}, 
           const std::vector<unsigned int> & = {0x00000000});
};


}
