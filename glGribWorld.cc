#include "glGribWorld.h"
#include "glGribOpengl.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glGribWorld::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glGribObject::clear ();
}

