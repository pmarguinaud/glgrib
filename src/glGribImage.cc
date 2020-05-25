#include "glGribImage.h"
#include "glGribBitmap.h"

#include <iostream>
#include <stdlib.h>


glGrib::Image::~Image ()
{
  clear ();
}

void glGrib::Image::clear ()
{
  if (ready)
    VAID.clear ();
}

glGrib::Image & glGrib::Image::operator= (const glGrib::Image & img)
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

void glGrib::Image::setupVertexAttributes () const
{
  // We have no buffer at all, but for some reason, we have to define a vertex array
  // so that the shader work. I do not know why.
}

void glGrib::Image::setup (const glGrib::OptionsImage & o)
{
  unsigned char * rgb;
  int w, h;

  opts = o;

  glGrib::Bitmap (opts.path, &rgb, &w, &h);

  texture = newGlgribOpenGLTexturePtr (w, h, rgb);
  delete [] rgb;

  setupVertexAttributes ();

  ready = true;
}

void glGrib::Image::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  glGrib::Program * program = glGrib::Program::load ("IMAGE");
  program->use ();

  program->set ("MVP", MVP);
  program->set ("x0", opts.x0);
  program->set ("y0", opts.y0);
  program->set ("x1", opts.x1);
  program->set ("y1", opts.y1);

  VAID.bind ();

  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set ("texture", 0); 

  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind);

  VAID.unbind ();

}



