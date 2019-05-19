#include "glgrib_world.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void glgrib_world::def_from_vertexbuffer_col_elementbuffer 
  (const glgrib_opengl_buffer_ptr colorbuffer, const_glgrib_geometry_ptr geom)
{
  numberOfPoints = geom->numberOfPoints;
  numberOfTriangles = geom->numberOfTriangles;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geom->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  if (colorbuffer && colorbuffer->allocated ())
    {
      colorbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 
    }

  geom->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 
}


