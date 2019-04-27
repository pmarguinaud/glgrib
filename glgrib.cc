#include "glgrib_x11.h"
#include "glgrib_fb.h"

#include "glgrib_options.h"

int main (int argc, char * argv[])
{
  glgrib_options opts;

  opts.parse (argc, argv);  

#ifdef USE_GLE
  fb_display (opts);
#else
  x11_display (opts);
#endif
  return 0;
}

