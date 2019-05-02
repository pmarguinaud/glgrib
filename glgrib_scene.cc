#include "glgrib_scene.h"
#include "glgrib_opengl.h"

#include <stdio.h>


using namespace glm;

void glgrib_scene::display_obj (const glgrib_object * obj) const
{
  const glgrib_program * program = obj->get_program ();
  program->use ();
  view.setMVP (program->matrixID);
  obj->render (&view);
}

void glgrib_scene::display () const
{
  view.calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  hidden.end ();
  if (landscape && (hidden.find (landscape) == hidden.end ()))
    display_obj (landscape);

  if (coastlines && (hidden.find (coastlines) == hidden.end ()))
    display_obj (coastlines);

  if (grid && (hidden.find (grid) == hidden.end ()))
    display_obj (grid);

  for (std::vector<glgrib_field*>::const_iterator it = fieldlist.begin (); 
       it != fieldlist.end (); it++)
    if (*it && (hidden.find (*it) == hidden.end ()))
      display_obj (*it);

}


