#pragma once

#include "glgrib_opengl.h"
#include "glgrib_handle.h"
#include "glgrib_options.h"
#include "glgrib_view.h"
#include "glgrib_program.h"

#include <glm/glm.hpp>
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

class glgrib_geometry;

typedef std::shared_ptr<glgrib_geometry> glgrib_geometry_ptr;
typedef std::shared_ptr<const glgrib_geometry> const_glgrib_geometry_ptr;

class glgrib_geometry
{
public:
  static glgrib_geometry_ptr load (class glgrib_loader *, const std::string &, const glgrib_options_geometry & opts, const int  = 0);
  virtual void getPointNeighbours (int, std::vector<int> *) const = 0;
  virtual float getLocalMeshSize (int) const = 0;
  virtual bool isEqual (const glgrib_geometry &) const = 0;
  virtual bool operator== (const glgrib_geometry & geom) const
  {
    return isEqual (geom);
  }
  virtual void setup (glgrib_handle_ptr, const glgrib_options_geometry &) = 0;
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual void index2latlon (int, float *, float *) const = 0;
  virtual ~glgrib_geometry ();
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
  virtual void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const = 0;
  virtual void renderTriangles () const;
  int getNumberOfPoints () const
  {
    return numberOfPoints;
  }
  int getNumberOfTriangles () const
  {
    return numberOfTriangles;
  }

  void bindTriangles () const
  {
    elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  }
  virtual void getView (glgrib_view *) const = 0;

  void bindCoordinates (int) const;

  void bindFrame (int attr = -1) const
  {
    vertexbuffer_frame->bind (GL_ARRAY_BUFFER);
    if (attr >= 0)
      {
        for (int j = 0; j < 3; j++)
          {
            glEnableVertexAttribArray (attr + j);
            glVertexAttribPointer (attr + j, 3, GL_FLOAT, GL_FALSE, 0, 
                                   (const void *)(j * 3 * sizeof (float)));
            glVertexAttribDivisor (attr + j, 1);
          }
      }
  }
  virtual int getFrameNumberOfPoints () const
  {
    return numberOfPoints_frame;
  }

  virtual void setProgramParameters (glgrib_program * program) const;

protected:
  unsigned int ind_strip_size = 0;
  int numberOfPoints = 0;
  unsigned int numberOfTriangles = 0;
  glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
  int numberOfPoints_frame = 0;
  glgrib_opengl_buffer_ptr vertexbuffer_frame;
  std::string md5string (const unsigned char []) const;
};

