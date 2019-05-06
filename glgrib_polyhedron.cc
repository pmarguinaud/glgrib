#include "glgrib_polyhedron.h"
#include "glgrib_opengl.h"

#include <stdlib.h>
#include <stdio.h>


void glgrib_polyhedron::def_from_xyz_col_ind (const float * xyz, unsigned char * col, unsigned int * ind)
{

  vertexbuffer  = new_opengl_buffer_ptr (3 * np * sizeof (float), xyz);
  colorbuffer   = new_opengl_buffer_ptr (ncol * np * sizeof (unsigned char), col);
  elementbuffer = new_opengl_buffer_ptr (3 * nt * sizeof (unsigned int), ind);

  //
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer->id ());
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer->id ());
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL); 
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer->id ());

  glBindVertexArray (0);
}

void glgrib_polyhedron::render (const glgrib_view * view) const
{
  glBindVertexArray (VertexArrayID);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
  if (wireframe)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

glgrib_polyhedron::~glgrib_polyhedron ()
{
  glDeleteVertexArrays (1, &VertexArrayID);
}

