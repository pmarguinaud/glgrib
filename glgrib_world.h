#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"

class world_t : public polyhedron_t
{
public:
  void init (const char *);
};

#endif
