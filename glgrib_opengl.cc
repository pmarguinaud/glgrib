#include "glgrib_opengl.h"

void gl_init ()
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS); 
}
  
glgrib_opengl_buffer::glgrib_opengl_buffer (size_t size, const void * data)
{
  glGenBuffers (1, &id_);
  glBindBuffer (GL_ARRAY_BUFFER, id_);
  glBufferData (GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  allocated_ = true;
}

glgrib_opengl_buffer::~glgrib_opengl_buffer ()
{
  if (allocated_)
    glDeleteBuffers (1, &id_);
  allocated_ = false;
}

glgrib_opengl_buffer_ptr new_glgrib_opengl_buffer_ptr (size_t size, const void * data)
{
  return std::make_shared<glgrib_opengl_buffer>(size, data);
}

glgrib_opengl_texture::glgrib_opengl_texture (int width, int height, const void * data)
{
  glGenTextures (1, &id_);
  glBindTexture (GL_TEXTURE_2D, id_);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); 
  glBindTexture (GL_TEXTURE_2D, 0); 
  allocated_ = true;
}

glgrib_opengl_texture::~glgrib_opengl_texture ()
{
  if (allocated_)
    glDeleteTextures (1, &id_);
  allocated_ = false;
}

glgrib_opengl_texture_ptr new_glgrib_opengl_texture_ptr (int width, int height, const void * data)
{
  return std::make_shared<glgrib_opengl_texture>(width, height, data);
}

