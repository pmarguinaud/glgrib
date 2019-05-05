#include "glgrib_x11.h"
#include "glgrib_options.h"

int main (int argc, char * argv[])
{
  glgrib_options opts;
  opts.parse (argc, argv);  
  x11_display (opts);
  return 0;
}

