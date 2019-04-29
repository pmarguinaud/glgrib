#ifndef _GLGRIB_GEOMETRY_GAUSSIAN_H
#define _GLGRIB_GEOMETRY_GAUSSIAN_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <glm/glm.hpp>
#include <eccodes.h>


class glgrib_geometry_gaussian : public glgrib_geometry
{
public:
  glgrib_geometry_gaussian (const glgrib_options &, codes_handle *);
  virtual ~glgrib_geometry_gaussian ();
  long int * pl = NULL;
  long int Nj;
  double stretchingFactor = 1.0f;
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  float omc2;
  float opc2;
  glm::mat4 rot = glm::mat4 (1.0f);
};

#endif
