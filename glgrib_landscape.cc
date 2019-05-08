#include "glgrib_landscape.h"
#include "glgrib_program.h"
#include "glgrib_bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

glgrib_landscape & glgrib_landscape::operator= (const glgrib_landscape & landscape)
{
  glgrib_world::operator= (landscape);
  ready_ = false;

  std::cout << " glgrib_landscape::operator= " << std::endl;

  texture = landscape.texture;
  flat    = landscape.flat;
  def_from_vertexbuffer_col_elementbuffer (NULL, geometry);

  setReady ();
}

void glgrib_landscape::init (const glgrib_options & opts, const glgrib_geometry_ptr geom)
{
  unsigned char * rgb;
  int w, h;

  geometry = geom;

  glgrib_bmp (opts.landscape.path.c_str (), &rgb, &w, &h);

  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);

  free (rgb);

  def_from_vertexbuffer_col_elementbuffer (NULL, geom);
  
  setReady ();
}

void glgrib_landscape::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program (); 
  glUniform1i (glGetUniformLocation (program->programID, "isflat"), flat);

  // the texture selection process is a bit obscure
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  glUniform1i (glGetUniformLocation (program->programID, "texture"), 0);

  glgrib_world::render (view);
}

glgrib_landscape::~glgrib_landscape ()
{
}





