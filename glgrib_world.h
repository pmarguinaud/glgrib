#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"
#include "glgrib_coords_world.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void def_from_vertexbuffer_col_elementbuffer 
          (const glgrib_coords_world *, unsigned char *);
};

#endif
