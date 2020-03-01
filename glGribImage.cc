#include "glGribImage.h"
#include "glGribBitmap.h"

#include <iostream>
#include <stdlib.h>


glGribImage::~glGribImage ()
{
  clear ();
}

void glGribImage::clear ()
{
  if (ready)
    glDeleteVertexArrays (1, &VertexArrayID);
}

glGribImage & glGribImage::operator= (const glGribImage & img)
{
  if (this != &img)
    {   
      clear (); 
      if (img.ready)
        {
          opts = img.opts;
          texture = img.texture;
          setupVertexAttributes (); 
          ready = true;
        }
    }   
  return *this;
}

void glGribImage::setupVertexAttributes ()
{

  // We have no buffer at all, but for some reason, we have to define a vertex array
  // so that the shader work. I do not know why.
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  glBindVertexArray (0);

}

void glGribImage::setup (const glGribOptionsImage & o)
{
  unsigned char * rgb;
  int w, h;

  opts = o;

  glGribBitmap (opts.path, &rgb, &w, &h);

  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);
  delete [] rgb;

  setupVertexAttributes ();

  ready = true;
}

void glGribImage::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  glGribProgram * program = glGribProgram::load (glGribProgram::IMAGE);
  program->use ();

  program->set ("MVP", MVP);
  program->set ("x0", opts.x0);
  program->set ("y0", opts.y0);
  program->set ("x1", opts.x1);
  program->set ("y1", opts.y1);

  glBindVertexArray (VertexArrayID);

  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set ("texture", 0); 

  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind);

  glBindVertexArray (0);

}



