#ifndef _GLGRIB_SHAPELIB_H
#define _GLGRIB_SHAPELIB_H


#include "glgrib_options.h"

#include <vector>
#include <string>

namespace glgrib_shapelib
{
extern void read (const glgrib_options_lines &, int *, unsigned int *, 
                  std::vector<float> *, std::vector<unsigned int> *,
                  const std::string &);
};

#endif
