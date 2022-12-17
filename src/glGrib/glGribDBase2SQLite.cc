#include "glGrib/DBase.h"

int main (int argc, char * argv[])
{
  glGrib::DBase d;

  d.convert2sqlite (argv[1]);

  return 0;
}

