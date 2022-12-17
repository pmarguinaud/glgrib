#pragma once


#include "glGrib/Options.h"

#include <vector>
#include <string>

namespace glGrib
{

namespace ShapeLib
{
void read (const OptionsLines &, 
           std::vector<float> *, std::vector<unsigned int> *,
           const std::string &);
};

}
