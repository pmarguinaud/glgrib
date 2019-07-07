#ifndef _GLGRIB_GEOMETRY_GAUSSIAN_H
#define _GLGRIB_GEOMETRY_GAUSSIAN_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <glm/glm.hpp>
#include <eccodes.h>

class glgrib_geometry_gaussian : public glgrib_geometry
{
public:
  static const double rad2deg;
  static const double deg2rad;
  virtual bool isEqual (const glgrib_geometry &) const;
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_gaussian (codes_handle *);
  glgrib_geometry_gaussian (int);
  virtual void init (codes_handle *, const float = 0.0f);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_gaussian ();
  virtual void applyUVangle (float *) const;
  virtual void sample (unsigned char *, const unsigned char, const int) const;
  virtual float resolution (int level = 0) const { if (level == 0) level = Nj; return M_PI / level; }
  virtual void getTriangleVertices (int, int [3]) const;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 xyz[3]) const;
  virtual bool triangleIsEdge (int) const;

private:
  class jlonlat_t
  {
  public:
    jlonlat_t () : jlon (0), jlat (0) {}
    jlonlat_t (int _jlon, int _jlat) : jlon (_jlon), jlat (_jlat) {}
    int jlon = 0;
    int jlat = 0;
  };

  jlonlat_t jlonlat (int) const;
  glm::vec3 jlonlat2xyz (const jlonlat_t & jlonlat) const
  {
    int jlat = jlonlat.jlat;
    int jlon = jlonlat.jlon;
    float coordy = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
    float sincoordy = sin (coordy);
    float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
    float coslat = cos (lat); float sinlat = sin (lat);
    float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
    float lon = coordx;
    float coslon = cos (lon); float sinlon = sin (lon);
    return glm::vec3 (coslat * coslon, coslat * sinlon, sinlat);
  }

  long int * pl = NULL;
  long int Nj;
  int * jglooff = NULL;
  double stretchingFactor = 1.0f;
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  float omc2 = 0.0f;
  float opc2 = 2.0f;
  glm::mat4 rot = glm::mat4 (1.0f);
  bool rotated = false;
  friend class sampler;
  unsigned int * ind = NULL;
  int * triu = NULL;          // Rank of triangle above
  int * trid = NULL;          // Rank of triangle below
};

#endif
