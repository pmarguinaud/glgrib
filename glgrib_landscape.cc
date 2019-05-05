#include "glgrib_landscape.h"
#include "glgrib_program.h"
#include "glgrib_bmp.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>

void glgrib_landscape::init (const glgrib_options & opts, const glgrib_geometry * geom)
{

  unsigned char * rgb;
  int w, h;

  glgrib_bmp (opts.landscape.c_str (), &rgb, &w, &h);

  glGenTextures (1, &textureID);
  glBindTexture (GL_TEXTURE_2D, textureID);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb); 
  glBindTexture (GL_TEXTURE_2D, 0);

  free (rgb);

  def_from_vertexbuffer_col_elementbuffer (NULL, geom);
  
}

void glgrib_landscape::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program (); 
  glUniform1i (glGetUniformLocation (program->programID, "isflat"), flat);
  glUniform1i (glGetUniformLocation (program->programID, "light"), light);

  // the texture selection process is a bit obscure
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, textureID);
  glUniform1i (glGetUniformLocation (program->programID, "texture"), 0);

  if (light)
    {
      const double deg2rad = M_PI / 180.0;
      float coslon = cos (deg2rad * lightx);
      float sinlon = sin (deg2rad * lightx);
      float coslat = cos (deg2rad * lighty);
      float sinlat = sin (deg2rad * lighty);
      float lightDir[3] = {coslon * coslat, sinlon * coslat, sinlat};
      glUniform3fv (glGetUniformLocation (program->programID, "lightDir"), 
                    1, &lightDir[0]);
    }

  glgrib_world::render (view);
}

glgrib_landscape::~glgrib_landscape ()
{
  glDeleteTextures (1, &textureID);
}





