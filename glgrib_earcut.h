#pragma once

#include <vector>

namespace glgrib_earcut
{
void processRing (const std::vector<float> &, 
                  int, int, int, int *,
                  std::vector<unsigned int> *, bool);
}

