#ifndef _GLGRIB_GEOMETRY_LATLON_H
#define _GLGRIB_GEOMETRY_LATLON_H

#include "glgrib_geometry.h"
#include "glgrib_trigonometry.h"
#include "glgrib_options.h"
#include "glgrib_handle.h"

class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  virtual bool isEqual (const glgrib_geometry &) const override;
  virtual void getPointNeighbours (int, std::vector<int> *) const override;
  virtual std::string md5 () const override;
  virtual int latlon2index (float, float) const override;
  virtual void index2latlon (int, float *, float *) const override;
  glgrib_geometry_latlon (glgrib_handle_ptr);
  virtual void setup (glgrib_handle_ptr, const glgrib_options_geometry &) override;
  virtual int size () const override;
  virtual ~glgrib_geometry_latlon ();
  virtual void applyNormScale (float *) const override {}
  virtual void applyUVangle (float *) const override {}
  virtual void sample (unsigned char *, const unsigned char, const int) const override;
  virtual void sampleTriangle (unsigned char *, const unsigned char, const int) const override;
  virtual float resolution (int level = 0) const override 
  { 
    if (level == 0)
      level = Nj;
    return deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees) / level;
  }
  virtual void getTriangleVertices (int, int [3]) const override;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const override;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const override;
  virtual bool triangleIsEdge (int) const override;
  virtual int getTriangle (float, float) const override;
  virtual glm::vec2 xyz2conformal (const glm::vec3 &) const override;
  virtual glm::vec3 conformal2xyz (const glm::vec2 &) const override;
  virtual glm::vec2 conformal2latlon (const glm::vec2 &) const override;
  virtual void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const override;
  virtual float getLocalMeshSize (int) const override;
  virtual void getView (glgrib_view *) const override;
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
