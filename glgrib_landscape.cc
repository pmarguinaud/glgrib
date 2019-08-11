#include "glgrib_landscape.h"
#include "glgrib_program.h"
#include "glgrib_bitmap.h"
#include "glgrib_resolve.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

glgrib_landscape & glgrib_landscape::operator= (const glgrib_landscape & landscape)
{
  cleanup ();
  if ((this != &landscape) && landscape.isReady ())
    {
      glgrib_world::operator= (landscape);
      texture = landscape.texture;
      flat    = landscape.flat;
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

void glgrib_landscape::init (glgrib_loader * ld, const glgrib_options_landscape & opts)
{
  unsigned char * rgb;
  int w, h;

  wireframe = opts.wireframe;

  geometry = glgrib_geometry_load (ld, opts.geometry, opts.orography, opts.number_of_latitudes);

  glgrib_bitmap (glgrib_resolve (opts.path), &rgb, &w, &h);

  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);

  free (rgb);

  setupVertexAttributes ();
  
  setReady ();
}

void glgrib_landscape::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::FLAT_TEX);
  program->use ();

  view.setMVP (program);
  program->setLight (light);
  program->set1i ("isflat", flat);

  // the texture selection process is a bit obscure
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set1i ("texture", 0);

  glgrib_world::render (view, light);
}

glgrib_landscape::~glgrib_landscape ()
{
}





