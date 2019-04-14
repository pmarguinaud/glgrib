#ifndef _GLGRIB_CUBE1_H
#define _GLGRIB_CUBE1_H

#include "glgrib_polyhedron.h"

class glgrib_cube1 : public glgrib_polyhedron
{
public:
  void init ();
  virtual int get_program_kind () const { return 1; }
  virtual bool use_alpha () { return false; }
};

#endif
