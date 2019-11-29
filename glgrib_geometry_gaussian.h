#ifndef _GLGRIB_GEOMETRY_GAUSSIAN_H
#define _GLGRIB_GEOMETRY_GAUSSIAN_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"
#include "glgrib_handle.h"

#include <glm/glm.hpp>
#include <vector>

class glgrib_geometry_gaussian : public glgrib_geometry
{
public:
  static const double rad2deg;
  static const double deg2rad;
  virtual bool isEqual (const glgrib_geometry &) const;
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_gaussian (glgrib_handle_ptr);
  glgrib_geometry_gaussian (int);
  virtual void setup (glgrib_handle_ptr, const glgrib_options_geometry &, const float = 0.0f);
  virtual int size () const;
  virtual ~glgrib_geometry_gaussian ();
  virtual void applyNormScale (float *) const;
  virtual void applyUVangle (float *) const;
  virtual void sample (unsigned char *, const unsigned char, const int) const;
  virtual void sampleTriangle (unsigned char *, const unsigned char, const int) const;
  virtual float resolution (int level = 0) const { if (level == 0) level = Nj; return M_PI / level; }
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 xyz[3]) const;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const;
  virtual bool triangleIsEdge (int) const;
  virtual int getTriangle (float, float) const;
  virtual glm::vec2 xyz2conformal (const glm::vec3 &) const;
  virtual glm::vec3 conformal2xyz (const glm::vec2 &) const;
  virtual glm::vec2 conformal2latlon (const glm::vec2 &) const;
  virtual void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const;

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
  glm::vec2 jlonlat2merc (const jlonlat_t & jlonlat) const
  {
    int jlat = jlonlat.jlat;
    int jlon = jlonlat.jlon;
    float coordy = latgauss[jlat-1];
    float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
    return glm::vec2 (coordx, log (tan (M_PI / 4.0f + coordy / 2.0f)));
  }
  glm::vec3 jlonlat2xyz (const jlonlat_t & jlonlat) const
  {
    int jlat = jlonlat.jlat;
    int jlon = jlonlat.jlon;
    float coordy = latgauss[jlat-1];
    float sincoordy = sin (coordy);
    float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
    float coslat = cos (lat); float sinlat = sin (lat);
    float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
    float lon = coordx;
    float coslon = cos (lon); float sinlon = sin (lon);

    float X = coslon * coslat;
    float Y = sinlon * coslat;
    float Z =          sinlat;

    if (! rotated)
      return glm::vec3 (X, Y, Z);
   
    glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
    XYZ = rot * XYZ;
 
    return glm::vec3 (XYZ.x, XYZ.y, XYZ.z);
  }

  int getUpperTriangle (int jglo, const jlonlat_t & jlonlat) const;
  
  int getLowerTriangle (int jglo, const jlonlat_t & jlonlat) const;

  void getTriangleVertices (int it, int jglo[3]) const;

  void getTriangleNeighbours (int, int [3], int [3], jlonlat_t [3]) const;
  void latlon2coordxy (float, float, float &, float &) const;
  int latlon2jlatjlon (float, float, int &, int &) const;

  int computeLowerTriangle (int, int) const;
  int computeUpperTriangle (int, int) const;
  void computeTriangleVertices (int, int [3]) const;
  void checkTriangleComputation () const;
  

private:
  std::vector<long int> pl;
  long int Nj;
  std::vector<int> jglooff;
  double stretchingFactor = 1.0f;
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  float omc2 = 0.0f;
  float opc2 = 2.0f;
  glm::mat4 rot = glm::mat4 (1.0f);
  bool rotated = false;
  // Keep the following as pointers for performance
  unsigned int * ind = NULL;

  int * indcnt_per_lat = NULL;
  int * indoff_per_lat = NULL;

  int * triu = NULL;          // Rank of triangle above
  int * trid = NULL;          // Rank of triangle below
  double * latgauss = NULL;
};

#endif
