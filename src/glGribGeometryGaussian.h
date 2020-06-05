#pragma once

#include "glGribGeometry.h"
#include "glGribTrigonometry.h"
#include "glGribOptions.h"
#include "glGribHandle.h"
#include "glGribOpenGL.h"
#include "glGribBuffer.h"

#include <glm/glm.hpp>
#include <vector>

namespace glGrib
{

class GeometryGaussian : public Geometry
{
public:
  bool isEqual (const Geometry &) const override;
  void getPointNeighbours (int, std::vector<int> *) const override;
  std::string md5 () const override;
  int latlon2index (float, float) const override;
  void index2latlon (int, float *, float *) const override;
  GeometryGaussian (HandlePtr);
  GeometryGaussian (int);
  void setup (HandlePtr, const OptionsGeometry &) override;
  int size () const override;
  virtual ~GeometryGaussian ();
  void applyNormScale (glGrib::BufferPtr<float> &) const override;
  void applyUVangle (glGrib::BufferPtr<float> &) const override;
  void sample (unsigned char *, const unsigned char, const int) const override;
  void sampleTriangle (unsigned char *, const unsigned char, const int) const override;
  float resolution (int level = 0) const override { if (level == 0) level = Nj; return M_PI / level; }
  void getTriangleNeighbours (int, int [3], int [3], glm::vec3 xyz[3]) const override;
  void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const override;
  bool triangleIsEdge (int) const override;
  int getTriangle (float, float) const override;
  glm::vec2 xyz2conformal (const glm::vec3 &) const override;
  glm::vec3 conformal2xyz (const glm::vec2 &) const override;
  glm::vec2 conformal2latlon (const glm::vec2 &) const override;
  void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const override;
  float getLocalMeshSize (int) const override;
  void getView (View *) const override;
  void setProgramParameters (Program *) const override;

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
   
    return rot * glm::vec3 (X, Y, Z);
  }
  glm::vec2 jlonlat2lonlat (const jlonlat_t & jlonlat) const
  {
    if ((stretchingFactor == 1.0f) && (! rotated))
      {
        int jlat = jlonlat.jlat, jlon = jlonlat.jlon;
        float lat = latgauss[jlat-1];
        float lon = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
        return glm::vec2 (lon, lat);
      }
    glm::vec3 xyz = jlonlat2xyz (jlonlat);
    float lon = atan2 (xyz.y, xyz.x);
    float lat = asin (xyz.z);
    return glm::vec2 (lon, lat);
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
  void setupSSBO ();
  void setupCoordinates ();
  void setupFitLatitudes ();

  class latfit_t
  {
  public:
    int kind = 0;
    std::vector<float> coeff;
    float error = 0;
  };

  void tryFitLatitudes (int, latfit_t *);

private:

  int kind = 0;
  std::vector<float> latfitcoeff;
  std::vector<long int> pl;
  long int Nj;
  std::vector<int> jglooff;
  double stretchingFactor = 1.0f;
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  float omc2 = 0.0f;
  float opc2 = 2.0f;
  glm::mat3 rot = glm::mat3 (1.0f);
  bool rotated = false;
  // Keep the following as pointers for performance
  unsigned int * ind = nullptr;

  int * indcnt_per_lat = nullptr;
  int * indoff_per_lat = nullptr;

  int * triu = nullptr;          // Rank of triangle above
  int * trid = nullptr;          // Rank of triangle below
  double * latgauss = nullptr;
  OpenGLBufferPtr<int> ssbo_jglo, ssbo_jlat;
  OpenGLBufferPtr<float> ssbo_glat;
};


}
