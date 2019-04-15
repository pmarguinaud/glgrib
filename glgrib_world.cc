#include "glgrib_world.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_coords_world.h"

#include <stdlib.h>
#include <stdio.h>

void glgrib_world::def_from_vertexbuffer_col_elementbuffer 
  (const glgrib_coords_world * coords, unsigned char * col)
{
  np = coords->np;
  nt = coords->nt;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, coords->vertexbuffer);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL); 
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, coords->elementbuffer);

  glBindVertexArray (0); 
}


