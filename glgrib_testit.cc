#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_options.h"
#include "glgrib_loader.h"

#include <iostream>


int main (int argc, const char * argv[])
{
  glgrib_options opts;

  glgrib_container * cont = glgrib_container::create (argv[1]);

  cont->buildIndex ();
  for (auto it = cont->begin (); it != cont->end (); ++it)
    {
      std::cout << *it << std::endl;
    }

  return 0;
}


