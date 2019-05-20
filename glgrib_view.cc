#include "glgrib_view.h"
#include "glgrib_opengl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <iostream>


void glgrib_view::setMVP (GLuint matrixID) const
{
  glUniformMatrix4fv (matrixID, 1, GL_FALSE, &MVP[0][0]);
}

void glgrib_view::calcMVP () const
{
  float xc = opts.distance * glm::cos (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)), 
        yc = opts.distance * glm::sin (glm::radians (opts.lon)) * glm::cos (glm::radians (opts.lat)),
        zc = opts.distance *                                      glm::sin (glm::radians (opts.lat));

  

  Viewport   = glm::vec4 (0.0f, 0.0f, (float)width, (float)height);

  float ratio = (float)width/(float)height;

  glm::mat4 Trans = glm::mat4 (1.0f);
  if (ratio > 1.0f)
    Trans = glm::translate (Trans, glm::vec3 ((ratio - 1.0f) / 2.0f, 0.0f, 0.0f));
   
  Projection = Trans * glm::perspective (glm::radians (opts.fov), (float)width/(float)height, 0.1f, 100.0f);
  View       = glm::lookAt (glm::vec3 (xc,yc,zc), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  Model      = glm::mat4 (1.0f);

  MVP = Projection * View * Model; 

}

void glgrib_view::setViewport (int w, int h)
{
  width = w;
  height = h;
}

glm::vec3 glgrib_view::insersect_plane (const double & xpos, const double & ypos, 
		                        const glm::vec3 & p, const glm::vec3 & v) const
{
// The plane is defined by the normal v and p which belongs to the plane

  glm::vec3 pa = glm::vec3 (xpos, ypos, +0.985601f);
  glm::vec3 pb = glm::vec3 (xpos, ypos, +0.900000f);
  glm::vec3 xa = unproject (pa);
  glm::vec3 xb = unproject (pb);

  glm::vec3 u = xb - xa;

  float lambda = glm::dot (v, p - xa) / glm::dot (u, v);

  return xa + lambda * u;
}

glm::vec3 glgrib_view::insersect_sphere (const double & xpos, const double & ypos, 
		                         const glm::vec3 & c, const float & r) const
{
// The sphere is defined by the radius r and its centre c

  glm::vec3 pa = glm::vec3 (xpos, ypos, +0.985601f);
  glm::vec3 pb = glm::vec3 (xpos, ypos, +0.900000f);
  glm::vec3 xa = unproject (pa);
  glm::vec3 xb = unproject (pb);

  glm::vec3 u = xb - xa;
  glm::vec3 dx = xa - c;

  float A = glm::dot (u, u);
  float B = 2. * glm::dot (dx, u);
  float C = dot (dx, dx) - r * r;
  float Delta = B * B - 4. * A * C;

  if (Delta < 0)
    return c;

  float lambda = (-B + sqrt (Delta)) / (2. * A);

  return xa + lambda * u;
}

