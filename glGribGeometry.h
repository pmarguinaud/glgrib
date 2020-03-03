#pragma once

#include "glGribOpenGL.h"
#include "glGribHandle.h"
#include "glGribOptions.h"
#include "glGribView.h"
#include "glGribProgram.h"

#include <glm/glm.hpp>
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

class glGribGeometry;

typedef std::shared_ptr<glGribGeometry> glGribGeometryPtr;
typedef std::shared_ptr<const glGribGeometry> const_glGribGeometryPtr;

class glGribGeometry
{
public:
  static glGribGeometryPtr load (class glGribLoader *, const std::string &, const glGribOptionsGeometry & opts, const int  = 0);
  virtual void getPointNeighbours (int, std::vector<int> *) const = 0;
  virtual float getLocalMeshSize (int) const = 0;
  virtual bool isEqual (const glGribGeometry &) const = 0;
  virtual bool operator== (const glGribGeometry & geom) const
  {
    return isEqual (geom);
  }
  virtual void setup (glGribHandlePtr, const glGribOptionsGeometry &) = 0;
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual void index2latlon (int, float *, float *) const = 0;
  virtual ~glGribGeometry ();
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
  virtual void getView (glGribView *) const = 0;

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

  virtual void setProgramParameters (glGribProgram * program) const;

protected:
  glGribOptionsGeometry opts;
  unsigned int ind_strip_size = 0;
  int numberOfPoints = 0;
  unsigned int numberOfTriangles = 0;
  glGribOpenGLBufferPtr vertexbuffer, elementbuffer;
  int numberOfPoints_frame = 0;
  glGribOpenGLBufferPtr vertexbuffer_frame;
  std::string md5string (const unsigned char []) const;
};

