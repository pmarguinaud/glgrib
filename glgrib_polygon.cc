#include "glgrib_polygon.h"
#include "glgrib_opengl.h"

#include <stdio.h>

void glgrib_polygon::def_from_xyz_col_ind 
  (glgrib_opengl_buffer_ptr vertexbuffer, 
   glgrib_opengl_buffer_ptr colorbuffer, 
   glgrib_opengl_buffer_ptr elementbuffer)
{
#ifdef UNDEF
  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz);
  if (col != NULL)
    colorbuffer = new_glgrib_opengl_buffer_ptr (ncol * numberOfPoints * sizeof (unsigned char), col);
  elementbuffer = new_glgrib_opengl_buffer_ptr (2 * nl * sizeof (unsigned int), ind);
#endif

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer->id ());
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 

  if (colorbuffer)
    {
      glBindBuffer (GL_ARRAY_BUFFER, colorbuffer->id ());
      glEnableVertexAttribArray (1); 
      glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL);
    }

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer->id ());

  glBindVertexArray (0);
}

void glgrib_polygon::render (const glgrib_view * view) const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_LINES, 2 * nl, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
}

glgrib_polygon::~glgrib_polygon ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
}

