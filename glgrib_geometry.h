#ifndef _GLGRIB_GEOMETRY_H
#define _GLGRIB_GEOMETRY_H

#include "glgrib_opengl.h"

#include <glm/glm.hpp>
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

class glgrib_geometry
{
public:
  class sampler
  {
  public:
    virtual int index () const  = 0;
    virtual bool next () = 0;
    virtual bool defined () const = 0;
    virtual float getUVangle () const = 0;
  protected:
    int level = 1;
  };

  virtual bool isEqual (const glgrib_geometry &) const = 0;
  virtual bool operator== (const glgrib_geometry & geom) const
  {
    return isEqual (geom);
  }
  virtual void init (codes_handle *, const float = 0.0f) = 0;
  virtual void genlatlon (float *, float *) const = 0;
  virtual void gencoords (float *, float *) const = 0;
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual ~glgrib_geometry ();
  int numberOfPoints; 
  unsigned int numberOfTriangles;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  virtual std::string md5 () const = 0;
  virtual sampler * newSampler (const int) const = 0;
protected:
  std::string md5string (const unsigned char []) const;
};

typedef std::shared_ptr<glgrib_geometry> glgrib_geometry_ptr;
typedef std::shared_ptr<const glgrib_geometry> const_glgrib_geometry_ptr;
extern glgrib_geometry_ptr glgrib_geometry_load (const std::string &, const float = 0.0f, const int  = 0);

#endif
