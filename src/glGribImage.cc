#include "glGribImage.h"
#include "glGribBitmap.h"

#include <iostream>
#include <stdlib.h>


void glGrib::Image::setup (const glGrib::OptionsImage & o)
{
  if (! o.on)
    return;

  unsigned char * rgb;
  int w, h;

  opts = o;

  glGrib::Bitmap (opts.path, &rgb, &w, &h);

  texture = glGrib::OpenGLTexturePtr (w, h, rgb);
  delete [] rgb;

  setupVertexAttributes ();

  setReady ();
}

void glGrib::Image::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
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



