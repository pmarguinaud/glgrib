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

  class sampler: public glgrib_geometry::sampler
  {
  public:
    sampler (const int lev, const glgrib_geometry_gaussian * g) : geom (g) { level = lev; }
    virtual float getUVangle () const
    {
      if (geom->rotated)
        {
          float coordy = M_PI * (0.5 - (float)(jlat + 1) / (float)(geom->Nj + 1));
          float sincoordy = sin (coordy);
          float lat = asin ((geom->omc2 + sincoordy * geom->opc2) / (geom->opc2 + sincoordy * geom->omc2));
          float coslat = cos (lat); float sinlat = sin (lat);
          float coordx = 2. * M_PI * (float)jlon / (float)geom->pl[jlat];
          float lon = coordx;
          float coslon = cos (lon); float sinlon = sin (lon);

          glm::vec4 XYZ = geom->rot * glm::vec4 (coslon * coslat, sinlon * coslat, sinlat, 0.0f);
          glm::vec3 xyz = glm::vec3 (XYZ.x, XYZ.y, XYZ.z);

          float coslon0 = cos (deg2rad * geom->longitudeOfStretchingPoleInDegrees);
          float sinlon0 = sin (deg2rad * geom->longitudeOfStretchingPoleInDegrees);
          float coslat0 = cos (deg2rad * geom->latitudeOfStretchingPoleInDegrees);
          float sinlat0 = sin (deg2rad * geom->latitudeOfStretchingPoleInDegrees);
       
          glm::vec3 xyz0 = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);
          glm::vec3 u1 = glm::normalize (glm::cross (xyz0 - xyz, xyz));
          glm::vec3 u0 = glm::normalize (glm::cross (glm::vec3 (0.0f, 0.0f, 1.0f), xyz));
          glm::vec3 v0 = glm::cross (xyz, u0);
          float u0u1 = glm::dot (u0, u1);
          float v0u1 = glm::dot (v0, u1);

          return rad2deg * atan2 (v0u1, u0u1);
        }
      return 0.0f;
    }
    virtual int index () const
    {
      return jglo;
    }
    virtual bool next ()
    {
      jglo++;
      jlon++;
      if (jlon >= geom->pl[jlat])
        {
          jlon = 0;
	  jlat++;
	}
      return jlat < geom->Nj;
    }
    virtual bool defined () const
    {
      int latlevel = (float)geom->Nj / (float)level;
      float lat = M_PI * (0.5 - (float)(jlat+1) / (float)(geom->Nj + 1));
      int lonlevel = latlevel / (2 * cos (lat));
      if (jlat % latlevel != 0)
        return false;
      if (jlon % lonlevel != 0)
        return false;
      return true;
    }
  private:
    const glgrib_geometry_gaussian * geom = NULL;
    int jglo = 0;
    int jlat = 0;
    int jlon = 0;
  };
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
  virtual glgrib_geometry::sampler * newSampler (const int level) const
  {
    return new sampler (level, this);
  }
private:
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
};

#endif
