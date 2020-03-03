#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>


namespace glGrib
{

class OpenGLBuffer
{
public:
  OpenGLBuffer (size_t, const void * = nullptr);
  ~OpenGLBuffer ();
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

typedef std::shared_ptr<OpenGLBuffer> OpenGLBufferPtr;
OpenGLBufferPtr newGlgribOpenGLBufferPtr (size_t, const void * = nullptr);
OpenGLBufferPtr newGlgribOpenGLBufferPtr (const OpenGLBufferPtr &);

class OpenGLTexture
{
public:
  OpenGLTexture (int, int, const void *);
  ~OpenGLTexture ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
private:
  bool allocated_ = false;
  GLuint id_;
};

typedef std::shared_ptr<OpenGLTexture> OpenGLTexturePtr;
OpenGLTexturePtr newGlgribOpenGLTexturePtr (const OpenGLTexturePtr &);
OpenGLTexturePtr newGlgribOpenGLTexturePtr (int, int, const void *);

void glInit ();

template <typename T> GLenum getOpenGLType ();
template <> GLenum getOpenGLType<unsigned char > ();
template <> GLenum getOpenGLType<unsigned short> ();
template <> GLenum getOpenGLType<unsigned int  > ();
template <> GLenum getOpenGLType<float         > ();


}
