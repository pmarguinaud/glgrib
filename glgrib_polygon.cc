#include "glgrib_polygon.h"
#include "glgrib_opengl.h"

#include <stdio.h>

void glgrib_polygon::def_from_xyz_col_ind 
  (glgrib_opengl_buffer_ptr vertexbuffer, 
   glgrib_opengl_buffer_ptr colorbuffer, 
   glgrib_opengl_buffer_ptr elementbuffer)
{

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 

  if (colorbuffer && colorbuffer->allocated ())
    {
      colorbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL);
    }

  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);

  glBindVertexArray (0);
}

void glgrib_polygon::render (const glgrib_view * view) const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_LINES, 2 * numberOfLines, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
}

glgrib_polygon::~glgrib_polygon ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
}

