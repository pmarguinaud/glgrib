#include "glgrib_world.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_world::def_from_vertexbuffer_col_elementbuffer 
  (unsigned char * col, const glgrib_geometry_ptr geom)
{
  np = geom->np;
  nt = geom->nt;

  colorbuffer = new_opengl_buffer_ptr (ncol * np * sizeof (unsigned char), col);

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, geom->vertexbuffer->id ());
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  if (col != NULL)
    {
      glBindBuffer (GL_ARRAY_BUFFER, colorbuffer->id ());
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL); 
    }

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, geom->elementbuffer->id ());
  glBindVertexArray (0); 
}


