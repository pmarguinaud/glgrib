#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_object.h"

class scene_t
{
public:
  void display () const;
  std::list<object_t*> objlist;
  view_t * view;
  program_t * prog;
};

#endif
