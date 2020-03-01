#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glGribOptions.h"
#include "glGribLoader.h"

#include <iostream>


int main (int argc, const char * argv[])
{
  glGribOptions opts;

  glGribContainer * cont = glGribContainer::create (argv[1]);

  cont->buildIndex ();
  for (auto it = cont->begin (); it != cont->end (); ++it)
    {
      std::cout << *it << std::endl;
    }

  return 0;
}


