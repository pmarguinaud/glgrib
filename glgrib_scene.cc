#include "glgrib_scene.h"
#include "glgrib_opengl.h"

#include <stdio.h>


using namespace glm;

void glgrib_scene::display () const
{
  view->calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  for (std::list<glgrib_object*>::const_iterator it = objlist.begin (); 
       it != objlist.end (); it++)
    if ((*it)->visible ())
      {
        const glgrib_program * program = (*it)->get_program ();
        program->use ();
        view->setMVP (program->matrixID);
        (*it)->render (view);
      }

}


