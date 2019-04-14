#ifndef _GLGRIB_OBJECT_H
#define _GLGRIB_OBJECT_H

#include "glgrib_opengl.h"

class glgrib_object
{
public:
  virtual void render () const = 0;
};

#endif
