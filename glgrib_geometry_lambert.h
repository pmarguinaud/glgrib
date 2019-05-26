#ifndef _GLGRIB_GEOMETRY_LAMBERT_H
#define _GLGRIB_GEOMETRY_LAMBERT_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <eccodes.h>


class glgrib_geometry_lambert : public glgrib_geometry
{
public:
  virtual bool isEqual (const glgrib_geometry &);
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_lambert (codes_handle *);
  virtual void init (codes_handle *, const float = 0.0f);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_lambert ();
private:
  long int Nx, Ny;
  double LaDInDegrees, LoVInDegrees, DxInMetres, DyInMetres;
};

#endif
