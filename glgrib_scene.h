#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <list>

#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_landscape_tex.h"
#include "glgrib_grid.h"
#include "glgrib_field.h"
#include "glgrib_coastlines.h"

class glgrib_scene
{
public:
  void display () const;
  void display_obj (const glgrib_object *) const;
  std::list<glgrib_object*> objlist;
  glgrib_view view;
  glgrib_landscape_tex * landscape = NULL;
  glgrib_field * field = NULL;
  glgrib_coastlines * coastlines = NULL;
  glgrib_grid * grid = NULL;
  void setGrid (glgrib_grid * g) { grid = g; }
  void setField (glgrib_field * f) { field = f; }
  void setCoastlines (glgrib_coastlines * c) { coastlines = c; }
  void setLandscape (glgrib_landscape_tex * l) { landscape = l; }
};

#endif
