#ifndef _GLGRIB_GEOMETRY_LATLON_H
#define _GLGRIB_GEOMETRY_LATLON_H

#include "glgrib_geometry.h"
#include "glgrib_options.h"

#include <eccodes.h>


class glgrib_geometry_latlon : public glgrib_geometry
{
public:
  class sampler: public glgrib_geometry::sampler
  {
  public:
    sampler (const int lev, const glgrib_geometry_latlon * g) : geom (g) { level = lev; }
    virtual float getUVangle () const 
    {
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
      if (jlon >= geom->Ni)
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
    const glgrib_geometry_latlon * geom = NULL;
    int jglo = 0;
    int jlat = 0;
    int jlon = 0;
  };
  virtual bool isEqual (const glgrib_geometry &) const;
  virtual std::string md5 () const;
  virtual int latlon2index (float, float) const;
  glgrib_geometry_latlon (codes_handle *);
  virtual void init (codes_handle *, const float = 0.0f);
  virtual void genlatlon (float *, float *) const;
  virtual void gencoords (float *, float *) const;
  virtual int size () const;
  virtual ~glgrib_geometry_latlon ();
  virtual glgrib_geometry::sampler * newSampler (const int level) const
  {
    return new sampler (level, this);
  }
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
