#include "glgrib_polyhedron.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glgrib_polyhedron::render (const glgrib_view * view) const
{
  glBindVertexArray (VertexArrayID);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

void glgrib_polyhedron::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
}

