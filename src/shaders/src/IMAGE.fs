
#include "version.h"

in 
#include "IMAGE_VS.h"

out vec4 color;

uniform sampler2D tex;

void main ()
{
  color = texture (tex, image_vs.fragmentTex);
}
