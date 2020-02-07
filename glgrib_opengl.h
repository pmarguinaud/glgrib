#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


class glgrib_opengl_buffer
{
public:
  glgrib_opengl_buffer (size_t, const void * = NULL);
  ~glgrib_opengl_buffer ();
  GLuint id () { return id_; }
  bool allocated () const { return allocated_; }
  void bind (GLenum) const;
  size_t buffersize () const { return size_; }
  void * map ();
  void unmap ();
private:
  bool allocated_ = false;
  GLuint id_;
  size_t size_;
};

typedef std::shared_ptr<glgrib_opengl_buffer> glgrib_opengl_buffer_ptr;
extern glgrib_opengl_buffer_ptr new_glgrib_opengl_buffer_ptr (size_t, const void * = NULL);
extern glgrib_opengl_buffer_ptr new_glgrib_opengl_buffer_ptr (const glgrib_opengl_buffer_ptr &);

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
extern glgrib_opengl_texture_ptr new_glgrib_opengl_texture_ptr (const glgrib_opengl_texture_ptr &);
extern glgrib_opengl_texture_ptr new_glgrib_opengl_texture_ptr (int, int, const void *);

void gl_init ();

template <typename T> GLenum getOpenglType ();
template <> GLenum getOpenglType<unsigned char > ();
template <> GLenum getOpenglType<unsigned short> ();
template <> GLenum getOpenglType<unsigned int  > ();
template <> GLenum getOpenglType<float         > ();

