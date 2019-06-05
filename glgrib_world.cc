#include "glgrib_world.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glgrib_world::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glBindVertexArray (VertexArrayID);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

void glgrib_world::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::cleanup ();
}

