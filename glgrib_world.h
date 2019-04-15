#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void init (const char *);
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_RGB; }
  virtual bool use_alpha () { return false; }
};

#endif
