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
  explicit GeometryGaussian (HandlePtr);
  explicit GeometryGaussian (int);

  bool isEqual (const Geometry &) const override;
  void getPointNeighbours (int, std::vector<int> *) const override;
  int latlon2index (float, float) const override;
  void index2latlon (int, float *, float *) const override;
  int size () const override;
  void applyNormScale (glGrib::BufferPtr<float> &) const override;
  void applyUVangle (glGrib::BufferPtr<float> &) const override;
  void sample (OpenGLBufferPtr<unsigned char> &, const unsigned char, const int) const override;
  void sampleTriangle (BufferPtr<unsigned char> &, const unsigned char, const int) const override;
  float resolution (int level = 0) const override 
  { 
    if (level == 0) 
    level = g.Nj; 
    return M_PI / level; 
  }
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
  void setup (HandlePtr, const OptionsGeometry &) override;
  std::string md5 () const override;

  class jlonlat_t
  {
  public:
    jlonlat_t () : jlon (0), jlat (0) {}
    explicit jlonlat_t (int _jlon, int _jlat) : jlon (_jlon), jlat (_jlat) {}
    int jlon = 0;
    int jlat = 0;
  };

  jlonlat_t jlonlat (int) const;
  glm::vec2 jlonlat2merc (const jlonlat_t & jlonlat) const
  {
    int jlat = jlonlat.jlat;
    int jlon = jlonlat.jlon;
    float coordy = d.latgauss[jlat-1];
    float coordx = 2. * M_PI * (float)(jlon-1) / (float)g.pl[jlat-1];
    return glm::vec2 (coordx, std::log (std::tan (M_PI / 4.0f + coordy / 2.0f)));
  }
  glm::vec3 jlonlat2xyz (const jlonlat_t & jlonlat) const
  {
    int jlat = jlonlat.jlat;
    int jlon = jlonlat.jlon;
    float coordy = d.latgauss[jlat-1];
    float sincoordy = std::sin (coordy);
    float lat = std::asin ((d.omc2 + sincoordy * d.opc2) / (d.opc2 + sincoordy * d.omc2));
    float coslat = std::cos (lat); float sinlat = std::sin (lat);
    float coordx = 2. * M_PI * (float)(jlon-1) / (float)g.pl[jlat-1];
    float lon = coordx;
    float coslon = std::cos (lon); float sinlon = std::sin (lon);

    float X = coslon * coslat;
    float Y = sinlon * coslat;
    float Z =          sinlat;

    if (! d.rotated)
      return glm::vec3 (X, Y, Z);
   
    return d.rot * glm::vec3 (X, Y, Z);
  }
  glm::vec2 jlonlat2lonlat (const jlonlat_t & jlonlat) const
  {
    if ((d.stretchingFactor == 1.0f) && (! d.rotated))
      {
        int jlat = jlonlat.jlat, jlon = jlonlat.jlon;
        float lat = d.latgauss[jlat-1];
        float lon = 2. * M_PI * (float)(jlon-1) / (float)g.pl[jlat-1];
        return glm::vec2 (lon, lat);
      }
    glm::vec3 xyz = jlonlat2xyz (jlonlat);
    float lon = atan2 (xyz.y, xyz.x);
    float lat = std::asin (xyz.z);
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

  GeometryGaussian () {}
  void triangulate ();

  void setupSubGrid ();

private:
  class GeometryGaussian * subgrid = nullptr;

  // Grid
  struct
  {
    std::vector<long int> pl;
    long int Nj = 0;
    std::vector<int> jglooff;
    BufferPtr<unsigned int> ind;
    BufferPtr<int> indcnt_per_lat;
    BufferPtr<int> indoff_per_lat;
    BufferPtr<int> triu; // Rank of triangle above
    BufferPtr<int> trid; // Rank of triangle below
  } g;

  // Coordinates
  struct
  {
    int kind = 0; // 1=linear, 2=octahedral
    std::vector<float> latfitcoeff;
    double stretchingFactor = 1.0f;
    double latitudeOfStretchingPoleInDegrees = 90.0f;
    double longitudeOfStretchingPoleInDegrees = 0.0f;
    float omc2 = 0.0f;
    float opc2 = 2.0f;
    glm::mat3 rot = glm::mat3 (1.0f);
    bool rotated = false;
    BufferPtr<double> latgauss;
    OpenGLBufferPtr<int> ssbo_jglo, ssbo_jlat;
    OpenGLBufferPtr<float> ssbo_glat;
  } d;
};


}
