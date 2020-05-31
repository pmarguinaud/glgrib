#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribGeometry.h"

namespace glGrib
{

class World : public Object
{
public:
  virtual ~World () { clear (); }
  const_GeometryPtr getGeometry () const { return geometry; }
  virtual void clear ();
protected:
  const_GeometryPtr geometry;
};


}
