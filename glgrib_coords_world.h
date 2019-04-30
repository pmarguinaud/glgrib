#ifndef _GLGRIB_COORDS_WORLD_H
#define _GLGRIB_COORDS_WORLD_H

#include "glgrib_opengl.h"
#include "glgrib_coords.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_coords_world : public glgrib_coords
{
public:
  void init (const glgrib_options &, const glgrib_geometry *);
};

#endif
