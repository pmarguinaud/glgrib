#include "glGribDbase.h"

int main (int argc, char * argv[])
{
  glGribDbase d;

  d.convert2sqlite (argv[1]);

  return 0;
}

