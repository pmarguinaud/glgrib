#include "glgrib_world.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glgrib_world::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
}

void glgrib_world::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::cleanup ();
}

