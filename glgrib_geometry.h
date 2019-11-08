#ifndef _GLGRIB_GEOMETRY_H
#define _GLGRIB_GEOMETRY_H

#include "glgrib_opengl.h"
#include "glgrib_handle.h"

#include <glm/glm.hpp>
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

class glgrib_geometry
{
public:
  virtual bool isEqual (const glgrib_geometry &) const = 0;
  virtual bool operator== (const glgrib_geometry & geom) const
  {
    return isEqual (geom);
  }
  virtual void setup (glgrib_handle_ptr, const float = 0.0f) = 0;
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual ~glgrib_geometry ();
  int numberOfPoints; 
  unsigned int numberOfTriangles;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  virtual std::string md5 () const = 0;
  virtual void applyUVangle (float *) const = 0;
  virtual void applyNormScale (float *) const = 0;
  virtual void sample (unsigned char *, const unsigned char, const int) const = 0;
  virtual void sampleTriangle (unsigned char *, const unsigned char, const int) const = 0;
  virtual float resolution (int = 0) const = 0;
  virtual void getTriangleVertices (int, int [3]) const = 0;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const = 0;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const = 0;
  virtual bool triangleIsEdge (int) const = 0;
  virtual int getTriangle (float, float) const = 0;
  virtual glm::vec2 xyz2conformal (const glm::vec3 &) const = 0;
  virtual glm::vec3 conformal2xyz (const glm::vec2 &) const = 0;
  virtual glm::vec2 conformal2latlon (const glm::vec2 &) const = 0;
  virtual void checkTriangles () const;
protected:
  std::string md5string (const unsigned char []) const;
};

typedef std::shared_ptr<glgrib_geometry> glgrib_geometry_ptr;
typedef std::shared_ptr<const glgrib_geometry> const_glgrib_geometry_ptr;
extern glgrib_geometry_ptr glgrib_geometry_load (class glgrib_loader *, const std::string &, const float = 0.0f, const int  = 0);

#endif
