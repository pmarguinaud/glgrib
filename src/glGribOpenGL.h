#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
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

template <typename T>
class OpenGLVertexArray
{
public:
  OpenGLVertexArray (T * _object) : object (_object) {}
  OpenGLVertexArray & operator= (const OpenGLVertexArray & other)
  {
    if (this != &other) 
      cleanup ();
    return *this;
  }
  void cleanup ()
  { 
    if (ready)
      glDeleteVertexArrays (1, &VertexArrayID);
    VertexArrayID = 0; 
    ready = false;
  }
  void setup ()
  {
    cleanup ();
    glGenVertexArrays (1, &VertexArrayID);
    ready = true;
  }
  void bind ()
  {
    if (! ready)
      throw std::runtime_error (std::string ("OpenGLVertexArray is not ready"));
    glBindVertexArray (VertexArrayID);
  }
private:
  T * object = nullptr;
  bool ready = false;
  GLuint VertexArrayID = 0;
};

void glInit ();

void glfwStart ();
void glfwStop ();

template <typename T> GLenum getOpenGLType ();
template <> GLenum getOpenGLType<unsigned char > ();
template <> GLenum getOpenGLType<unsigned short> ();
template <> GLenum getOpenGLType<unsigned int  > ();
template <> GLenum getOpenGLType<float         > ();


}
