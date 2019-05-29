#ifndef _GLGRIB_GEOMETRY_GAUSSIAN_H
#define _GLGRIB_GEOMETRY_GAUSSIAN_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <glm/glm.hpp>
#include <eccodes.h>

class glgrib_geometry_gaussian : public glgrib_geometry
{
public:
  class sampler: public glgrib_geometry::sampler
  {
  public:
    sampler (const int lev, const glgrib_geometry_gaussian * g) : geom (g) { level = lev; }
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
      if (jlat % level != 0)
        return false;
      if (jlon % level != 0)
        return false;
      return true;
    }
  private:
    const glgrib_geometry_gaussian * geom = NULL;
    int jglo = 0;
    int jlat = 0;
    int jlon = 0;
  };
  virtual bool isEqual (const glgrib_geometry &);
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
  friend class sampler;
};

#endif
