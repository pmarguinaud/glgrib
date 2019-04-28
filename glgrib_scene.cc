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
  
  if (landscape)
    display_obj (landscape);

  if (coastlines)
    display_obj (coastlines);

  if (grid)
    display_obj (grid);

  for (std::list<glgrib_object*>::const_iterator it = objlist.begin (); 
       it != objlist.end (); it++)
    if (*it)
      display_obj (*it);

  if (field)
    display_obj (field);


}


