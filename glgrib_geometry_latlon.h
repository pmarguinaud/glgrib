#ifndef _GLGRIB_GEOMETRY_LATLON_H
#define _GLGRIB_GEOMETRY_LATLON_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <eccodes.h>


class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_latlon (codes_handle *);
  virtual void init (codes_handle *, const glgrib_options * = NULL);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_latlon ();
  long int Ni, Nj;
  double latitudeOfFirstGridPointInDegrees;
  double longitudeOfFirstGridPointInDegrees;
  double latitudeOfLastGridPointInDegrees;
  double longitudeOfLastGridPointInDegrees;
};

#endif
