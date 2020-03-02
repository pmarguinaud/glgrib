#include "glGribTest.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"

#include <stdlib.h>
#include <stdio.h>



void glGribTest::render (const glGribView & view, const glGribOptionsLight & light) const
{
  return;
  glGribProgram * program = glGribProgram::load (glGribProgram::TEST);
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray (0);

  glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glGribTest::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glGribObject::clear ();
}

void glGribTest::setup ()
{
  return;
  int numberOfPoints = 3;

  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};


  numberOfTriangles = ind.size () / 3;

  vertexbuffer = newGlgribOpenGLBufferPtr (lonlat.size () * sizeof (lonlat[0]), lonlat.data ());
  elementbuffer = newGlgribOpenGLBufferPtr (ind.size () * sizeof (ind[0]), ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

