#include "glgrib_font.h"
#include "glgrib_shader.h"
#include "glgrib_bmp.h"


glgrib_font_ptr new_glgrib_font_ptr ()
{
  return std::make_shared<glgrib_font> ();
}

GLuint glgrib_font::programID = 0;
bool glgrib_font::programReady = false;

void glgrib_font::select () const
{
  glUseProgram (programID);
  glUniform1fv (glGetUniformLocation (programID, "xoff"), xoff.size (), xoff.data ());
  glUniform1fv (glGetUniformLocation (programID, "yoff"), yoff.size (), yoff.data ());
  glUniform1i (glGetUniformLocation (programID, "nx"), nx);
  glUniform1i (glGetUniformLocation (programID, "ny"), ny);
  glUniform1f (glGetUniformLocation (programID, "aspect"), aspect);
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture);
  glUniform1i (glGetUniformLocation (programID, "texture"), 0);

}

glgrib_font::~glgrib_font ()
{
  if (ready)
    glDeleteTextures (1, &texture);
}

void glgrib_font::init (const glgrib_options_font & o)
{
  opts = o;
  unsigned char * rgb = NULL;
  int w, h;
  std::vector<int> ioff, joff;

  glgrib_bmp (opts.bitmap.c_str (), &rgb, &w, &h);

  for (int i = 0, p = w * (h - 2); i < w; i++, p += 1)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        xoff.push_back ((float)(i+1)/(float)w);
        ioff.push_back (i+1);
      }

  for (int j = 0, p = 1; j < h; j++, p += w)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        yoff.push_back ((float)(j+1)/(float)h);
        joff.push_back (j+1);
      }

  for (int p = 0; p < w * h; p++)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      { 
        rgb[3*p+0] = 255;
	rgb[3*p+1] = 255; 
	rgb[3*p+2] = 255;
      }

  nx = xoff.size ()-1;
  ny = yoff.size ()-1;


  int rank, ix, iy, pp;

  // Look at nuber of pixels below letter 'i'
  rank = map ('i'); ix = rank % nx; iy = rank / nx;

  pp = 0;
  for (int j = joff[iy]; j < joff[iy+1]; j++)
    for (int i = ioff[ix]; i < ioff[ix+1]; i++)
      {
        int p = j * w + i;
        if (rgb[3*p+0] < 200)
          {
            pp = j - joff[iy];
            goto found_b;
          }
      }
found_b:
  posb = (float)pp / (float)(joff[1] - joff[0]);
  
  // Look at number of pixels above letter 'F'
  rank = map ('F'); ix = rank % nx; iy = rank / nx;

  pp = 0;
  for (int j = joff[iy+1]-1; j > joff[iy]; j--)
   for (int i = ioff[ix]; i < ioff[ix+1]; i++)
      {
        int p = j * w + i;
        if (rgb[3*p+0] < 200)
          {
            pp = joff[iy+1]-1 - j;
            goto found_u;
          }
      }
found_u:
  posu = (float)pp / (float)(joff[1] - joff[0]);

  
  aspect = ((float)w / (float)nx) / ((float)h / (float)ny);

  glGenTextures (1, &texture);
  glBindTexture (GL_TEXTURE_2D, texture); 
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // The third argument has to be GL_RED, but I do not understand why
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);

  loadShader ();

  free (rgb);
  ready = true;
}

void glgrib_font::loadShader ()
{
  if (programReady)
    return;

  programID = glgrib_load_shader
(
R"CODE(
#version 330 core

in vec2 fragmentPos;
in vec2 letterPos;
in float letterRank;

out vec4 color;

uniform sampler2D texture;

uniform float xoff[30];
uniform float yoff[30];
uniform int nx = 0;
uniform int ny = 0;
uniform float aspect = 1.0;
uniform float scale = 1.0;
uniform vec3 color0;

void main ()
{
  float dx = scale * aspect;
  float dy = scale;

  int ix = int (mod (letterRank, nx));
  int iy = int (letterRank / nx);

  float tx = (fragmentPos.x - letterPos.x) / dx;
  float ty = (fragmentPos.y - letterPos.y) / dy;

  tx = xoff[ix] + tx * (xoff[ix+1] - xoff[ix]);
  ty = yoff[iy] + ty * (yoff[iy+1] - yoff[iy]);

  vec4 col = texture2D (texture, vec2 (tx, ty));

  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1. - col.r;
   
}
)CODE",
R"CODE(

#version 330 core

layout (location = 0) in vec2 vertexPos;
layout (location = 1) in vec3 letterAtt;

out vec2 fragmentPos;
out vec2 letterPos;
out float letterRank;

uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (0., vertexPos.x, vertexPos.y, 1.);
  fragmentPos = vec2 (vertexPos.x, vertexPos.y);
  letterRank = letterAtt.z;
  letterPos  = vec2 (letterAtt.x, letterAtt.y);
}


)CODE");

}


