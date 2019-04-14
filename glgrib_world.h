#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void init (const char *);
//virtual int get_program_kind () { return 1; }
//virtual bool use_alpha () { return false; }
};

#endif
