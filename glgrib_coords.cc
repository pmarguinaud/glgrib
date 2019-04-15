#include "glgrib_coords.h"
#include "glgrib_opengl.h"

glgrib_coords::~glgrib_coords ()
{
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteBuffers (1, &elementbuffer);
}

