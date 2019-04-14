#include "glgrib_polygon.h"
#include "glgrib_opengl.h"

void glgrib_polygon::def_from_xyz_col_ind (const float * xyz, const unsigned char * col, const unsigned int * ind)
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 

  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL);

  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 2 * nl * sizeof (unsigned int), ind, GL_STATIC_DRAW);
}

void glgrib_polygon::render () const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_LINES, 2 * nl, GL_UNSIGNED_INT, NULL);
}

glgrib_polygon::~glgrib_polygon ()
{
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteBuffers (1, &colorbuffer);
  glDeleteVertexArrays (1, &VertexArrayID);
}

