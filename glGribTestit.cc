#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glGribOptions.h"
#include "glGribLoader.h"
#include "glGribContainer.h"

#include <iostream>


int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  glGrib::Container * cont = glGrib::Container::create (argv[1]);

  cont->buildIndex ();
  for (auto it = cont->begin (); it != cont->end (); ++it)
    {
      std::cout << *it << std::endl;
    }

  return 0;
}


