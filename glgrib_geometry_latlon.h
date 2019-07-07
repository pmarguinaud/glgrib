#ifndef _GLGRIB_GEOMETRY_LATLON_H
#define _GLGRIB_GEOMETRY_LATLON_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <eccodes.h>


class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  static const double rad2deg;
  static const double deg2rad;
  virtual bool isEqual (const glgrib_geometry &) const;
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_latlon (codes_handle *);
  virtual void init (codes_handle *, const float = 0.0f);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_latlon ();
  virtual void applyUVangle (float *) const {}
  virtual void sample (unsigned char *, const unsigned char, const int) const;
  virtual float resolution (int level = 0) const 
  { 
    if (level == 0)
      level = Nj;
    return deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees) / level;
  }
  virtual void getTriangleVertices (int, int [3]) const
  { throw std::runtime_error (std::string ("Not implemented")); }
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const
  { throw std::runtime_error (std::string ("Not implemented")); }
  virtual bool triangleIsEdge (int) const
  { throw std::runtime_error (std::string ("Not implemented")); }
private:
  long int Ni, Nj;
  double latitudeOfFirstGridPointInDegrees;
  double longitudeOfFirstGridPointInDegrees;
  double latitudeOfLastGridPointInDegrees;
  double longitudeOfLastGridPointInDegrees;
  double dlat, dlon, lat0, lon0;
  friend class sampler;
};

#endif
