#include "glgrib_options.h"

int main (int argc, char * argv[])
{
  glgrib_options opts;
  glgrib_options_parser oh;
  opts.traverse ("", &oh);
  oh.parse (argc, argv);
  return 0;
}
