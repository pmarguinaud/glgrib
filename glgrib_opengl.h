#ifndef _GLGRIB_OPENGL_H
#define _GLGRIB_OPENGL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


class glgrib_opengl_buffer
{
public:
  glgrib_opengl_buffer (size_t, const void *);
  ~glgrib_opengl_buffer ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
private:
  bool allocated_ = false;
  GLuint id_;
};

typedef std::shared_ptr<glgrib_opengl_buffer> glgrib_opengl_buffer_ptr;
extern glgrib_opengl_buffer_ptr new_glgrib_opengl_buffer_ptr (size_t, const void *);

class glgrib_opengl_texture
{
public:
  glgrib_opengl_texture (int, int, const void *);
  ~glgrib_opengl_texture ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
private:
  bool allocated_ = false;
  GLuint id_;
};

typedef std::shared_ptr<glgrib_opengl_texture> glgrib_opengl_texture_ptr;
extern glgrib_opengl_texture_ptr new_glgrib_opengl_texture_ptr (int, int, const void *);

void gl_init ();

#endif
