#include "glgrib_scene.h"
#include "glgrib_opengl.h"
#include "glgrib_window.h"

#include <stdio.h>


using namespace glm;

void glgrib_scene::display (glgrib_window * window) const
{
  window->makeCurrent ();
  view.calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  for (std::list<glgrib_object*>::const_iterator it = objlist.begin (); 
       it != objlist.end (); it++)
      {
        const glgrib_program * program = (*it)->get_program ();
        program->use ();
        view.setMVP (program->matrixID);
        (*it)->render (&view);
      }

}


