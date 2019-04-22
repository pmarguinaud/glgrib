#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "glgrib_x11.h"
#include "glgrib_fb.h"

int main (int argc, char * argv[])
{
//const int width = 1024, height = 1024;
  const int width = 800, height = 800;
  const char * geom = argv[1];
#ifdef USE_GLE
  fb_display (geom, width, height);
#else
  x11_display (geom, width, height);
#endif
  return 0;
}

