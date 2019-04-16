#include "glgrib_palette.h"

void glgrib_palette::setRGBA255 (GLuint programID)
{
  float R0[256], G0[256], B0[256], A0[256];
  int n = rgba.size ();

  for (int i = 0; i < n-1; i++)
    {
      int j0 = (255 * (i + 0)) / (n-1);
      int j1 = (255 * (i + 1)) / (n-1);
      R0[i] = rgba[i].r;
    }
    

  glUniform1fv (glGetUniformLocation (programID, "R0"), 256, R0);
  glUniform1fv (glGetUniformLocation (programID, "G0"), 256, G0);
  glUniform1fv (glGetUniformLocation (programID, "B0"), 256, B0);
  glUniform1fv (glGetUniformLocation (programID, "A0"), 256, A0);
}
