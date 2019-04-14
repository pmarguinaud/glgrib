#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

glgrib_program::~glgrib_program ()
{
  glDeleteProgram (programID);
}

void glgrib_program::init ()
{
  programID = glgrib_load_shader ();
}

