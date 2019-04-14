#include "glgrib_view.h"
#include "glgrib_opengl.h"

void view_t::init (prog_t * prog)
{
  MatrixID = glGetUniformLocation (prog->programID, "MVP");
}
