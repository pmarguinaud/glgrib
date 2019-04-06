#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "glgrib_x11.h"
#include "glgrib_fb.h"

int main (int argc, char * argv[])
{
  const int width = 1024, height = 1024;
  const char * file = argv[1];
  x11_display (file, width, height);
//fb_display (file, width, height);
  return 0;
}

