#include "glgrib_dbase.h"

int main (int argc, char * argv[])
{
  glgrib_dbase d;

  d.convert2sqlite (argv[1]);

  return 0;
}

