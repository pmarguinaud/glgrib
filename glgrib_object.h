#ifndef _GLGRIB_OBJECT_H
#define _GLGRIB_OBJECT_H

#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_program.h"

class glgrib_object
{
public:
  virtual void render (const glgrib_view *) const = 0;
  glgrib_program * program;
};

#endif
