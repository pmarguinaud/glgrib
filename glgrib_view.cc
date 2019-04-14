#include "glgrib_view.h"
#include "glgrib_opengl.h"

void glgrib_view::init (glgrib_program * prog)
{
  MatrixID = glGetUniformLocation (prog->programID, "MVP");
}
