#include "glgrib_view.h"
#include "glgrib_opengl.h"

void glgrib_view::init (GLuint programID)
{
  MatrixID = glGetUniformLocation (programID, "MVP");
}
