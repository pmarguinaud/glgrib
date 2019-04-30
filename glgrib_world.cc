#include "glgrib_world.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_world::def_from_vertexbuffer_col_elementbuffer 
  (unsigned char * col, const glgrib_geometry * geom)
{
  np = geom->np;
  nt = geom->nt;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, geom->vertexbuffer);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  if (col != NULL)
    {
      glGenBuffers (1, &colorbuffer);
      glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
      glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL); 
    }

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, geom->elementbuffer);
  glBindVertexArray (0); 
}


