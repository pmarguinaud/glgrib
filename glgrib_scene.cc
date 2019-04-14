#include "glgrib_scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glgrib_opengl.h"

using namespace glm;

void scene_t::display () const
{
  float xc = view->rc * glm::cos (glm::radians (view->lonc)) * glm::cos (glm::radians (view->latc)), 
        yc = view->rc * glm::sin (glm::radians (view->lonc)) * glm::cos (glm::radians (view->latc)),
        zc = view->rc *                                        glm::sin (glm::radians (view->latc));

  glm::mat4 Projection = glm::perspective (glm::radians (view->fov), 1.0f / 1.0f, 0.1f, 100.0f);
  glm::mat4 View       = glm::lookAt (glm::vec3 (xc,yc,zc), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 Model      = glm::mat4 (1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glUseProgram (prog->programID);
  glUniformMatrix4fv (view->MatrixID, 1, GL_FALSE, &MVP[0][0]);
  
  for (std::list<obj_t*>::const_iterator it = objlist.begin (); 
       it != objlist.end (); it++)
    (*it)->render ();

}

