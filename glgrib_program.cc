#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

program_t::~program_t ()
{
  glDeleteProgram (programID);
}

void program_t::init ()
{
  programID = glgrib_load_shader ();
}

