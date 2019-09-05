#include "glgrib_landscape.h"
#include "glgrib_program.h"
#include "glgrib_bitmap.h"
#include "glgrib_png.h"
#include "glgrib_resolve.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <exception>

#include <math.h>

glgrib_landscape & glgrib_landscape::operator= (const glgrib_landscape & landscape)
{
  clear ();
  if ((this != &landscape) && landscape.isReady ())
    {
      glgrib_world::operator= (landscape);
      texture = landscape.texture;
      opts    = landscape.opts;
      setupVertexAttributes ();
      setReady ();
   }
}

void glgrib_landscape::setupVertexAttributes ()
{
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  geometry->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 
}

static bool endsWith (const std::string & str, const std::string & end)
{
  int i0 = std::max (str.length () - end.length (), (size_t)0);
  int i1 = end.length ();
  return end == str.substr (i0, i1);
}

void glgrib_landscape::setup (glgrib_loader * ld, const glgrib_options_landscape & o)
{
  opts = o;

  unsigned char * rgb;
  int w, h;

  geometry = glgrib_geometry_load (ld, opts.geometry, opts.orography, opts.number_of_latitudes);

  if (endsWith (opts.path, ".png"))
    glgrib_read_png (glgrib_resolve (opts.path), &w, &h, &rgb);
  else if (endsWith (opts.path, ".bmp"))
    glgrib_bitmap (glgrib_resolve (opts.path), &rgb, &w, &h);
  else
    throw std::runtime_error (std::string ("Unknown image format :") + opts.path);

  GLint sizemax;
  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sizemax);
  if ((sizemax < w) || (sizemax < h))
    throw std::runtime_error (std::string ("Image is too large to be used as a texture :") + opts.path);


  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);

  free (rgb);

  setupVertexAttributes ();
  
  setReady ();
}

void glgrib_landscape::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  const float deg2rad = M_PI / 180.0;

  glgrib_program * program = glgrib_program_load (glgrib_program::FLAT_TEX);
  program->use ();

  view.setMVP (program);
  program->setLight (light);
  program->set1i ("isflat", opts.flat.on);

  // the texture selection process is a bit obscure
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set1i ("texture", 0);
  program->set1f ("lonA", opts.position.lon1 * deg2rad);
  program->set1f ("lonB", opts.position.lon2 * deg2rad);
  program->set1f ("latA", opts.position.lat1 * deg2rad);
  program->set1f ("latB", opts.position.lat2 * deg2rad);

  glBindVertexArray (VertexArrayID);
  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

}

glgrib_landscape::~glgrib_landscape ()
{
}





