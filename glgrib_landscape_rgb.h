#ifndef _GLGRIB_LANDSCAPE_RGB_H
#define _GLGRIB_LANDSCAPE_RGB_H

#include "glgrib_landscape.h"
#include "glgrib_coords_world.h"

#include <string>

class glgrib_landscape_rgb : public glgrib_landscape
{
public:
  virtual void init (const glgrib_options &, const glgrib_coords_world *);
};

#endif
