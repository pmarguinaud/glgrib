#include "glgrib_image.h"
#include "glgrib_bitmap.h"

#include <iostream>
#include <stdlib.h>


glgrib_image::~glgrib_image ()
{
  clear ();
}

void glgrib_image::clear ()
{
  if (ready)
    glDeleteVertexArrays (1, &VertexArrayID);
}

glgrib_image & glgrib_image::operator= (const glgrib_image & img)
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

void glgrib_image::setupVertexAttributes ()
{

  // We have no buffer at all, but for some reason, we have to define a vertex array
  // so that the shader work. I do not know why.
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  glBindVertexArray (0);

}

void glgrib_image::setup (const glgrib_options_image & o)
{
  unsigned char * rgb;
  int w, h;

  opts = o;

  glgrib_bitmap (opts.path, &rgb, &w, &h);

  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);
  delete [] rgb;

  setupVertexAttributes ();

  ready = true;
}

void glgrib_image::render (const glm::mat4 & MVP) const
{
  if (! ready)
    return;

  glgrib_program * program = glgrib_program::load (glgrib_program::IMAGE);
  program->use ();

  program->set ("MVP", MVP);
  program->set1f ("x0", opts.x0);
  program->set1f ("y0", opts.y0);
  program->set1f ("x1", opts.x1);
  program->set1f ("y1", opts.y1);

  glBindVertexArray (VertexArrayID);

  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set1i ("texture", 0); 

  unsigned int ind[6] = {0, 1, 2, 2, 3, 0};
  glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind);

  glBindVertexArray (0);

}



