#ifndef _GLGRIB_CUBE_H
#define _GLGRIB_CUBE_H

#include "glgrib_polyhedron.h"

class cube_t : public polyhedron_t
{
public:
  void init ();
  virtual void render () const;
};

#endif
