#include "glGrib/Image.h"
#include "glGrib/Bitmap.h"
#include "glGrib/Buffer.h"

#include <iostream>

namespace glGrib
{

void Image::setup (const OptionsImage & o)
{
  if (! o.on)
    return;

  BufferPtr<unsigned char> rgb;
  int w, h;

  opts = o;

  Bitmap (opts.path, rgb, &w, &h);

  texture = OpenGLTexturePtr (w, h, rgb);

  setReady ();
}

void Image::render (const glm::mat4 & MVP) const
{
  if (! isReady ())
    return;

  Program * program = Program::load ("IMAGE");
  program->use ();

  program->set ("MVP", MVP);
  program->set ("x0", opts.x0);
  program->set ("y0", opts.y0);
  program->set ("x1", opts.x1);
  program->set ("y1", opts.y1);

  VAID.bind ();

  texture->bind (0);
  program->set ("tex", 0); 

  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind);

  VAID.unbind ();

}

}

