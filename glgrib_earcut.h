#ifndef _GLGRIB_EARCUT_H
#define _GLGRIB_EARCUT_H

#include <vector>

namespace glgrib_earcut
{
void processRing (const std::vector<float> &, 
                  int, int, int, int *,
                  std::vector<unsigned int> *, bool);
}

#endif
