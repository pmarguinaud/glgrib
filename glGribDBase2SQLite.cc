#include "glGribDBase.h"

int main (int argc, char * argv[])
{
  glGribDBase d;

  d.convert2sqlite (argv[1]);

  return 0;
}

