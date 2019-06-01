#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"
#include "glgrib_geometry.h"
#include "glgrib_opengl.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  const_glgrib_geometry_ptr geometry;
};

#endif
