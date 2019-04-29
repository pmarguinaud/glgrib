#include "glgrib_coords_world.h"
#include "glgrib_load.h"

#include <math.h>
#include <stdlib.h>

void glgrib_coords_world::init (const glgrib_options & opts)
{
  unsigned int * ind = NULL;
  float * xyz = NULL;
  glgrib_load_z (opts.geometry.c_str (), &np, &xyz, &nt, &ind, opts.orography);

  glBindVertexArray (0);
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);

  free (ind);
  free (xyz);
}


