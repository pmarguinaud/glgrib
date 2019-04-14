#include "glgrib_cube.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>

#include "glgrib_opengl.h"

void glgrib_cube::render (const glgrib_view * view) const
{
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  // GL_LINES : avec 3 points, on fait juste un segment (le dernier point n'est pas pris)
  // GL_LINE_LOOP : avec 3 points, on fait un triangle ferme
  // GL_LINE_STRIP : avec 3 points, on fait un triangle ouvert
//glDrawElements (GL_LINES, 3 * 1, GL_UNSIGNED_INT, (void *)(0 * sizeof (GLuint))); 
//glDrawElements (GL_LINES, 36 * 1, GL_UNSIGNED_INT, (void *)(0 * sizeof (GLuint)));
//glDrawElements (GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(3 * sizeof (GLuint)));
//glDrawElements (GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(0));
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, (void *)(0));
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);

}


void glgrib_cube::init ()
{
  program = glgrib_program_load (0);

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  ncol = use_alpha ? 4 : 3;
  nt = 12;
  np = 8;

  float * xyz = (float *)malloc (3 * np * sizeof (float));
  unsigned char * col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
  unsigned int * ind = (unsigned int *)malloc (nt * 3 * sizeof (unsigned int));

  float s = 1.0;

  xyz[0*3+0] = -s; xyz[0*3+1] = -s; xyz[0*3+2] = -s;
  xyz[1*3+0] = -s; xyz[1*3+1] = +s; xyz[1*3+2] = -s;
  xyz[2*3+0] = -s; xyz[2*3+1] = +s; xyz[2*3+2] = +s;
  xyz[3*3+0] = -s; xyz[3*3+1] = -s; xyz[3*3+2] = +s;
  xyz[4*3+0] = +s; xyz[4*3+1] = -s; xyz[4*3+2] = -s;
  xyz[5*3+0] = +s; xyz[5*3+1] = +s; xyz[5*3+2] = -s;
  xyz[6*3+0] = +s; xyz[6*3+1] = +s; xyz[6*3+2] = +s;
  xyz[7*3+0] = +s; xyz[7*3+1] = -s; xyz[7*3+2] = +s;


  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 120;

  for (int i = 0; i < np; i++)
  for (int j = 0; j < 3; j++)
    col[ncol*i+j] = 255 * ((s + xyz[i*3+j]) / (2 * s));

  ind[ 0] = 4; ind[ 1] = 5; ind[ 2] = 6;
  ind[ 3] = 4; ind[ 4] = 6; ind[ 5] = 7;

  ind[ 6] = 1; ind[ 7] = 0; ind[ 8] = 2;
  ind[ 9] = 2; ind[10] = 0; ind[11] = 3;

  ind[12] = 1; ind[13] = 2; ind[14] = 6;
  ind[15] = 1; ind[16] = 6; ind[17] = 5;

  ind[18] = 2; ind[19] = 3; ind[20] = 6;
  ind[21] = 3; ind[22] = 7; ind[23] = 6;

  ind[24] = 3; ind[25] = 0; ind[26] = 4;
  ind[27] = 3; ind[28] = 4; ind[29] = 7;

  ind[30] = 5; ind[31] = 4; ind[32] = 1;
  ind[33] = 1; ind[34] = 4; ind[36] = 0;

  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);

  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);

  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind, GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);
}

