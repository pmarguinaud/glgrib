// Wrapper around the fixed copy of EarCut.cc, compiled with -DglGrib=glGribFix so that both
// versions of glGrib::EarCut::processRing can be linked in the same program.
#include "glGrib/EarCut.h"

void earcutFix (const std::vector<float> & lonlat, int rankb, int rankl, int indrb, int * indrl,
                std::vector<unsigned int> * ind, bool openmp)
{
  glGribFix::EarCut::processRing (lonlat, rankb, rankl, indrb, indrl, ind, openmp);
}
