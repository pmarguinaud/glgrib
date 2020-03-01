#pragma once

#include <vector>

namespace glGribEarcut
{
void processRing (const std::vector<float> &, 
                  int, int, int, int *,
                  std::vector<unsigned int> *, bool);
}

