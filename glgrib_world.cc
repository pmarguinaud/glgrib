#include "glgrib_world.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void glgrib_world::def_from_vertexbuffer_col_elementbuffer 
  (const glgrib_opengl_buffer_ptr colorbuffer, const glgrib_geometry_ptr geom)
{
  numberOfPoints = geom->numberOfPoints;
  numberOfTriangles = geom->numberOfTriangles;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, geom->vertexbuffer->id ());
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  if (colorbuffer && colorbuffer->allocated ())
    {
      glBindBuffer (GL_ARRAY_BUFFER, colorbuffer->id ());
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 
    }

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, geom->elementbuffer->id ());
  glBindVertexArray (0); 
}


