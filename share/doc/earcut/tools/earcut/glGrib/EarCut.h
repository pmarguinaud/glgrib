#pragma once

#include <vector>

namespace glGrib
{

namespace EarCut
{
void processRing (const std::vector<float> &, 
                  int, int, int, int *,
                  std::vector<unsigned int> *, bool);
}

}
