
#include "version.h"

in 
#include "IMAGE_VS.h"

out vec4 color;

uniform sampler2D texture;

void main ()
{
  color = texture2D (texture, image_vs.fragmentTex);
}
