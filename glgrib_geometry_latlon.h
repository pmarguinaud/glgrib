#pragma once

#include "glgrib_geometry.h"
#include "glgrib_trigonometry.h"
#include "glgrib_options.h"
#include "glgrib_handle.h"

class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  bool isEqual (const glgrib_geometry &) const override;
  void getPointNeighbours (int, std::vector<int> *) const override;
  std::string md5 () const override;
  int latlon2index (float, float) const override;
  void index2latlon (int, float *, float *) const override;
  glgrib_geometry_latlon (glgrib_handle_ptr);
  void setup (glgrib_handle_ptr, const glgrib_options_geometry &) override;
  int size () const override;
  virtual ~glgrib_geometry_latlon ();
  void applyNormScale (float *) const override {}
  void applyUVangle (float *) const override {}
  void sample (unsigned char *, const unsigned char, const int) const override;
  void sampleTriangle (unsigned char *, const unsigned char, const int) const override;
  float resolution (int level = 0) const override 
  { 
    if (level == 0)
      level = Nj;
    return deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees) / level;
  }
  void getTriangleVertices (int, int [3]) const override;
  void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const override;
  void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const override;
  bool triangleIsEdge (int) const override;
  int getTriangle (float, float) const override;
  glm::vec2 xyz2conformal (const glm::vec3 &) const override;
  glm::vec3 conformal2xyz (const glm::vec2 &) const override;
  glm::vec2 conformal2latlon (const glm::vec2 &) const override;
  void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const override;
  float getLocalMeshSize (int) const override;
  void getView (glgrib_view *) const override;
  void setProgramParameters (glgrib_program *) const override;
private:
  void setupCoordinates ();
  void setupFrame ();
  void getTriangleNeighboursLatLon (int, int [3], int [3], float &, float &, float &, float &) const;
  long int Ni, Nj;
  double latitudeOfFirstGridPointInDegrees;
  double longitudeOfFirstGridPointInDegrees;
  double latitudeOfLastGridPointInDegrees;
  double longitudeOfLastGridPointInDegrees;
  double dlat, dlon, lat0, lon0;
  bool periodic = false;
};

