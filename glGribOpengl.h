#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


class glGribOpenglBuffer
{
public:
  glGribOpenglBuffer (size_t, const void * = nullptr);
  ~glGribOpenglBuffer ();
  GLuint id () { return id_; }
  bool allocated () const { return allocated_; }
  void bind (GLenum, GLuint = 0) const;
  size_t buffersize () const { return size_; }
  void * map ();
  void unmap ();
private:
  bool allocated_ = false;
  GLuint id_;
  size_t size_;
};

typedef std::shared_ptr<glGribOpenglBuffer> glgrib_opengl_buffer_ptr;
extern glgrib_opengl_buffer_ptr newGlgribOpenglBufferPtr (size_t, const void * = nullptr);
extern glgrib_opengl_buffer_ptr newGlgribOpenglBufferPtr (const glgrib_opengl_buffer_ptr &);

class glGribOpenglTexture
{
public:
  glGribOpenglTexture (int, int, const void *);
  ~glGribOpenglTexture ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
private:
  bool allocated_ = false;
  GLuint id_;
};

typedef std::shared_ptr<glGribOpenglTexture> glgrib_opengl_texture_ptr;
extern glgrib_opengl_texture_ptr newGlgribOpenglTexturePtr (const glgrib_opengl_texture_ptr &);
extern glgrib_opengl_texture_ptr newGlgribOpenglTexturePtr (int, int, const void *);

void glInit ();

template <typename T> GLenum getOpenglType ();
template <> GLenum getOpenglType<unsigned char > ();
template <> GLenum getOpenglType<unsigned short> ();
template <> GLenum getOpenglType<unsigned int  > ();
template <> GLenum getOpenglType<float         > ();

