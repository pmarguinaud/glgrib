#ifndef _GLGRIB_OPENGL_H
#define _GLGRIB_OPENGL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


class opengl_buffer
{
public:
  opengl_buffer (size_t, const void *);
  ~opengl_buffer ();
  GLuint id () { return id_; }
private:
  bool allocated = false;
  GLuint id_;
};

typedef std::shared_ptr<opengl_buffer> opengl_buffer_ptr;
extern opengl_buffer_ptr new_opengl_buffer_ptr (size_t, const void *);

class opengl_texture
{
public:
  opengl_texture (int, int, const void *);
  ~opengl_texture ();
  GLuint id () { return id_; }
private:
  bool allocated = false;
  GLuint id_;
};

typedef std::shared_ptr<opengl_texture> opengl_texture_ptr;
extern opengl_texture_ptr new_opengl_texture_ptr (int, int, const void *);

void gl_init ();

#endif
