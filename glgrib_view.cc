#include "glgrib_view.h"
#include "glgrib_opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

using namespace glm;

void glgrib_view::init (glgrib_program * program)
{
  MatrixID = glGetUniformLocation (program->programID, "MVP");
}

void glgrib_view::setMVP (GLuint matrixID) const
{
  float xc = rc * glm::cos (glm::radians (lonc)) * glm::cos (glm::radians (latc)), 
        yc = rc * glm::sin (glm::radians (lonc)) * glm::cos (glm::radians (latc)),
        zc = rc *                                        glm::sin (glm::radians (latc));

  glm::mat4 Projection = glm::perspective (glm::radians (fov), 1.0f / 1.0f, 0.1f, 100.0f);
  glm::mat4 View       = glm::lookAt (glm::vec3 (xc,yc,zc), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 Model      = glm::mat4 (1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  
  glUniformMatrix4fv (matrixID, 1, GL_FALSE, &MVP[0][0]);
 
}
