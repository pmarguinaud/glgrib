#include <omp.h>

#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribBatch.h"
#include "glGribSnapshot.h"

#include <iostream>

#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <string.h>

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if (! opts.parse (argc, argv))
    return 1;

  glGrib::Batch batch (opts);

  glGrib::framebuffer (batch, "toto.png");

  return 0;
}


