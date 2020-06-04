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
protected:
  const_GeometryPtr geometry;
};


}
