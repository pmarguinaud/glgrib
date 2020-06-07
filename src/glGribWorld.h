#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribGeometry.h"

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
