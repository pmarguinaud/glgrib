#include "glgrib_prog.h"
#include "glgrib_opengl.h"
#include "shader.h"

prog_t::~prog_t ()
{
  glDeleteProgram (programID);
}

void prog_t::init ()
{
  programID = LoadShaders ();
}

