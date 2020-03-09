#include "glGribWorld.h"
#include "glGribOpenGL.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glGrib::World::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glGrib::Object::clear ();
}

