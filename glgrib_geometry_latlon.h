#ifndef _GLGRIB_GEOMETRY_LATLON_H
#define _GLGRIB_GEOMETRY_LATLON_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"
#include "glgrib_handle.h"

class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  static const double rad2deg;
  static const double deg2rad;
  virtual bool isEqual (const glgrib_geometry &) const;
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_latlon (glgrib_handle_ptr);
  virtual void setup (glgrib_handle_ptr, const glgrib_options_geometry &, const float = 0.0f);
  virtual int size () const;
  virtual ~glgrib_geometry_latlon ();
  virtual void applyNormScale (float *) const {}
  virtual void applyUVangle (float *) const {}
  virtual void sample (unsigned char *, const unsigned char, const int) const;
  virtual void sampleTriangle (unsigned char *, const unsigned char, const int) const;
  virtual float resolution (int level = 0) const 
  { 
    if (level == 0)
      level = Nj;
    return deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees) / level;
  }
  virtual void getTriangleVertices (int, int [3]) const;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const;
  virtual bool triangleIsEdge (int) const;
  virtual int getTriangle (float, float) const;
  virtual glm::vec2 xyz2conformal (const glm::vec3 &) const;
  virtual glm::vec3 conformal2xyz (const glm::vec2 &) const;
  virtual glm::vec2 conformal2latlon (const glm::vec2 &) const;
  virtual void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const;
private:
  void getTriangleNeighboursLatLon (int, int [3], int [3], float &, float &, float &, float &) const;
  long int Ni, Nj;
  double latitudeOfFirstGridPointInDegrees;
  double longitudeOfFirstGridPointInDegrees;
  double latitudeOfLastGridPointInDegrees;
  double longitudeOfLastGridPointInDegrees;
  double dlat, dlon, lat0, lon0;
  bool periodic = false;
};

#endif
