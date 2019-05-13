#ifndef _GLGRIB_GEOMETRY_H
#define _GLGRIB_GEOMETRY_H

#include "glgrib_options.h"
#include "glgrib_opengl.h"
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

class glgrib_geometry
{
public:
  virtual bool isEqual (const glgrib_geometry &) = 0;
  virtual bool operator== (const glgrib_geometry & geom)
  {
    return isEqual (geom);
  }
  virtual void init (codes_handle *, const glgrib_options * = NULL) = 0;
  virtual void genlatlon (float *, float *) const = 0;
  virtual void gencoords (float *, float *) const = 0;
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual ~glgrib_geometry ();
  int numberOfPoints; 
  unsigned int numberOfTriangles;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  virtual std::string md5 () const = 0;
protected:
  std::string md5string (const unsigned char []) const;
};

typedef std::shared_ptr<glgrib_geometry> glgrib_geometry_ptr;
extern glgrib_geometry_ptr glgrib_geometry_load (const std::string &, const glgrib_options * = NULL);

#endif
