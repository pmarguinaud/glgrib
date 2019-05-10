#ifndef _GLGRIB_GEOMETRY_GAUSSIAN_H
#define _GLGRIB_GEOMETRY_GAUSSIAN_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <glm/glm.hpp>
#include <eccodes.h>


class glgrib_geometry_gaussian : public glgrib_geometry
{
public:
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_gaussian (codes_handle *);
  virtual void init (codes_handle *, const glgrib_options * = NULL);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_gaussian ();
private:
  long int * pl = NULL;
  long int Nj;
  int * jglooff = NULL;
  double stretchingFactor = 1.0f;
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  float omc2;
  float opc2;
  glm::mat4 rot = glm::mat4 (1.0f);
};

#endif
