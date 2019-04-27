#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_landscape.h"
#include "glgrib_field.h"

class glgrib_scene
{
public:
  void display (class glgrib_window *) const;
  std::list<glgrib_object*> objlist;
  glgrib_view view;
  glgrib_landscape * landscape = NULL;
  glgrib_field * field = NULL;
};

#endif
