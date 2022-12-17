#pragma once

#include "glGrib/Object.h"
#include "glGrib/OpenGL.h"
#include "glGrib/View.h"
#include "glGrib/Geometry.h"

namespace glGrib
{

class World : public Object3D
{
public:
  const_GeometryPtr getGeometry () const { return geometry; }
  void setGeometry (const GeometryPtr & g) { geometry = g; }
private:
  const_GeometryPtr geometry;
};


}
