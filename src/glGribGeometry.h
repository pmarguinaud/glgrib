#pragma once

#include "glGribOpenGL.h"
#include "glGribHandle.h"
#include "glGribOptions.h"
#include "glGribView.h"
#include "glGribProgram.h"
#include "glGribBuffer.h"

#include <glm/glm.hpp>
#include <eccodes.h>
#include <memory>
#include <string>
#include <iostream>

namespace glGrib
{

class Geometry;

typedef std::shared_ptr<Geometry> GeometryPtr;
typedef std::shared_ptr<const Geometry> const_GeometryPtr;

class Geometry
{
public:
  static GeometryPtr load (class Loader *, const std::string &, const OptionsGeometry & opts, const int  = 0);
  static void clearCache ();
  virtual void setup (HandlePtr, const OptionsGeometry &) = 0;
  virtual void getPointNeighbours (int, std::vector<int> *) const = 0;
  virtual float getLocalMeshSize (int) const = 0;
  virtual bool isEqual (const Geometry &) const = 0;
  virtual bool operator== (const Geometry & geom) const
  {
    return isEqual (geom);
  }
  virtual int size () const = 0;
  virtual int latlon2index (float, float) const = 0;
  virtual void index2latlon (int, float *, float *) const = 0;
  virtual ~Geometry ();
  virtual void applyUVangle (glGrib::BufferPtr<float> &) const = 0;
  virtual void applyNormScale (glGrib::BufferPtr<float> &) const = 0;
  virtual void sample (OpenGLBufferPtr<unsigned char> &, const unsigned char, const int) const = 0;
  virtual void sampleTriangle (BufferPtr<unsigned char> &, const unsigned char, const int) const = 0;
  virtual float resolution (int = 0) const = 0;
  virtual void getTriangleVertices (int, int [3]) const = 0;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec3 [3]) const = 0;
  virtual void getTriangleNeighbours (int, int [3], int [3], glm::vec2 [3]) const = 0;
  virtual bool triangleIsEdge (int) const = 0;
  virtual int getTriangle (float, float) const = 0;
  virtual const glm::vec2 xyz2conformal (const glm::vec3 &) const = 0;
  virtual const glm::vec3 conformal2xyz (const glm::vec2 &) const = 0;
  virtual const glm::vec2 conformal2latlon (const glm::vec2 &) const = 0;
  virtual void checkTriangles () const;
  virtual void fixPeriodicity (const glm::vec2 &, glm::vec2 *, int) const = 0;
  virtual void renderTriangles (int level = 0) const;

  void widenRegion (const float, const int, glGrib::BufferPtr<float> &) const;

  int getNumberOfPoints () const
  {
    return grid.numberOfPoints;
  }

  int getNumberOfTriangles () const
  {
    return grid.numberOfTriangles;
  }

  void bindTriangles (int level = 0) const;

  virtual void getView (View *) const = 0;

  void bindCoordinates (int) const;

  bool hasFrame () const
  {
    return frame.vertexbuffer->size () > 0;
  }

  void bindFrame (int attr = -1) const
  {
    frame.vertexbuffer->bind (GL_ARRAY_BUFFER);
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

  int getFrameNumberOfPoints () const
  {
    return frame.numberOfPoints;
  }

  virtual void setProgramParameters (Program * program) const;

  const OpenGLBufferPtr<float> & getVertexBuffer () const
  {
    return crds.vertexbuffer;
  }

  void setOptions (const OptionsGeometry & o)
  {
    opts = o;
  }

  const OptionsGeometry & getOptions () const
  {
    return opts;
  }

  const OpenGLBufferPtr<unsigned int> & getElementBuffer () const
  {
    return grid.elementbuffer;
  }

private:
  const std::string md5string (const unsigned char []) const;
  virtual const std::string md5 () const = 0;
private:
  OptionsGeometry opts;

  class Geometry * subgrid = nullptr;

  // Grid
  struct
  {
    unsigned int numberOfTriangles = 0;
    int numberOfPoints = 0;
    OpenGLBufferPtr<unsigned int> elementbuffer;
    unsigned int ind_strip_size;
  } grid;

  // Coordinates
  struct
  {
    OpenGLBufferPtr<float> vertexbuffer;
  } crds;

  // Frame
  struct
  {
    int numberOfPoints = 0;
    OpenGLBufferPtr<float> vertexbuffer;
  } frame;

private:
  friend class GeometryLatLon;
  friend class GeometryLambert;
  friend class GeometryGaussian;
};


}
