#include "glgrib_coords_world.h"
#include "glgrib_load.h"

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

void glgrib_coords_world::init (const glgrib_options & opts, const glgrib_geometry * geom)
{
  unsigned int * ind = NULL;
  float * xyz = NULL;
  glgrib_load_z (opts.geometry.c_str (), &np, &xyz, &nt, &ind, opts.orography);

  if (np != geom->np)
    std::cout << " np != " << std::endl;
  if (nt != geom->nt)
    std::cout << " nt != " << std::endl;
  if (memcmp (ind, geom->ind, nt * sizeof (unsigned int)))
    std::cout << " ind != " << std::endl;
  if (memcmp (xyz, geom->xyz, np * sizeof (float)))
    std::cout << " xyz != " << std::endl;

  vertexbuffer = geom->vertexbuffer;
  elementbuffer = geom->elementbuffer;

#ifdef UNDEF
  glBindVertexArray (0);
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * geom->np * sizeof (float), geom->xyz, GL_STATIC_DRAW);
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * geom->nt * sizeof (unsigned int), geom->ind , GL_STATIC_DRAW);
#endif

  free (ind);
  free (xyz);
}


