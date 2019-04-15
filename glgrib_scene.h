#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_world.h"

class glgrib_scene
{
public:
  void display () const;
  std::list<glgrib_object*> objlist;
  glgrib_view * view = NULL;
  glgrib_world * world = NULL;
};

#endif
