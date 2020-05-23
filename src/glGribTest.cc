#include "glGribTest.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"

#include <stdlib.h>
#include <stdio.h>



void glGrib::Test::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  return;
  glGrib::Program * program = glGrib::Program::load ("TEST");
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  VAID.bind ();
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  VAID.unbind ();

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glGrib::Test::clear ()
{
  if (isReady ())
    VAID.clear ();
  glGrib::Object::clear ();
}

void glGrib::Test::setup ()
{
  return;

  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};


  numberOfTriangles = ind.size () / 3;

  vertexbuffer = newGlgribOpenGLBufferPtr (lonlat.size () * sizeof (lonlat[0]), lonlat.data ());
  elementbuffer = newGlgribOpenGLBufferPtr (ind.size () * sizeof (ind[0]), ind.data ());

  VAID.setup ();
  VAID.bind ();

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  VAID.unbind ();

  setReady ();
}

