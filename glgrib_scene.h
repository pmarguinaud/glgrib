#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_obj.h"

class scene_t
{
public:
  void display () const;
  std::list<obj_t*> objlist;
  view_t * view;
  prog_t * prog;
};

#endif
