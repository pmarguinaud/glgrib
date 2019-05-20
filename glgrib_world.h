#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"
#include "glgrib_geometry.h"
#include "glgrib_opengl.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void def_from_vertexbuffer_col_elementbuffer 
          (const glgrib_opengl_buffer_ptr, const_glgrib_geometry_ptr);
  const_glgrib_geometry_ptr geometry;
};

#endif
