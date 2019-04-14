#ifndef _GLGRIB_CUBE_H
#define _GLGRIB_CUBE_H

#include "glgrib_polyhedron.h"
#include "glgrib_view.h"

class glgrib_cube : public glgrib_polyhedron
{
public:
  void init ();
  virtual void render (const glgrib_view *) const;
};

#endif
