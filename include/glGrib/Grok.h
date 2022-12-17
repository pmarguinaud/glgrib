#pragma once

#include <string>

namespace glGrib
{

enum class grok_t
{
  UNKNOWN=0,
  LFI=1,
  GRIB=2,
  LFI_EXT=3,
  GRIB_EXT=4
};

grok_t Grok (const std::string &); 

}
