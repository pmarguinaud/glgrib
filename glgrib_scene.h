#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_object.h"

class glgrib_scene
{
public:
  void display () const;
  std::list<glgrib_object*> objlist;
  glgrib_view * view;
  GLuint programID;
};

#endif
