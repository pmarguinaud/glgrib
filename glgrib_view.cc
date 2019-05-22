#include "glgrib_view.h"
#include "glgrib_opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <iostream>

#include "glgrib_projection.h"

void glgrib_view::setMVP (GLuint programID) const
{
  GLuint matrixID = glGetUniformLocation (programID, "MVP");
  if (matrixID >= 0)
    glUniformMatrix4fv (matrixID, 1, GL_FALSE, &MVP[0][0]);
  GLuint projID = glGetUniformLocation (programID, "proj");
  if (projID >= 0)
    glUniform1i (projID, ps.current ()->getType ());
}

void glgrib_view::calcMVP () const
{
  glm::vec3 pos
    (opts.distance * glm::cos (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)), 
     opts.distance * glm::sin (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)),
     opts.distance *                                      glm::sin (glm::radians (opts.lat)));

  Viewport   = glm::vec4 (0.0f, 0.0f, (float)width, (float)height);

  float ratio = (float)width/(float)height;

  glm::mat4 Trans = glm::mat4 (1.0f);

  if (ratio > 1.0f)
    Trans = glm::translate (Trans, glm::vec3 ((ratio - 1.0f) / 2.0f, 0.0f, 0.0f));
       
  Projection = Trans * glm::perspective (glm::radians (opts.fov), ratio, 0.1f, 100.0f);
  View       = ps.current ()->getView (pos, opts.distance);
  Model      = glm::mat4 (1.0f);
  MVP = Projection * View * Model; 

}

void glgrib_view::setViewport (int w, int h)
{
  width = w;
  height = h;
}

int glgrib_view::get_latlon_from_screen_coords (float xpos, float ypos, float * lat, float * lon) const
{
  glm::vec3 pos;

  glm::vec3 xa = unproject (glm::vec3 (xpos, ypos, +0.985601f));
  glm::vec3 xb = unproject (glm::vec3 (xpos, ypos, +0.900000f));

  if (! ps.current ()->unproject (xa, xb, &pos))
    return 0;

  *lat = glm::degrees (glm::asin (pos.z));
  *lon = glm::degrees (glm::atan (pos.y, pos.x));

  return 1;
}


