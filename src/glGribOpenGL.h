#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <memory>
#include <iostream>


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
private:

  class binding_t
  {
  public:
    binding_t ()
    {
      glGetIntegerv (GL_VERTEX_ARRAY_BINDING, &VertexArrayID);
      printf (" current = %8d\n", VertexArrayID);
    }
    ~binding_t ()
    {
      
    }
  private:
    GLuint VertexArrayID = 0;
  };

  // Copy constructor should be managed by host object
  OpenGLVertexArray (const OpenGLVertexArray & other) 
  {
  }

public:

  OpenGLVertexArray (T * _object) : object (_object) {}

  OpenGLVertexArray & operator= (const OpenGLVertexArray & other)
  {
    if (this != &other) 
      clear ();
    return *this;
  }

  ~OpenGLVertexArray ()
  {
    clear ();
  }

  template <typename A0>
  void render (const A0 & a0) const
  {
    bind ();
    object->render (a0);
    unbind ();
  }

  void clear ()
  { 
    if (ready)
{
if (0){
printf ("0x%llx ", (long long unsigned int)glfwGetCurrentContext ());
std::cout << "glDeleteVertexArrays " << VertexArrayID << std::endl;
}
      glDeleteVertexArrays (1, &VertexArrayID);
}
    VertexArrayID = 0; 
    ready = false;
  }

  const T * getObject () const
  {
    return object;
  }

  void unbind () const
  {
    glBindVertexArray (0); 
  }

  void bind () const
  {
    if (! ready)
      {
        glGenVertexArrays (1, &VertexArrayID);
if (0){
printf ("0x%llx ", (long long unsigned int)glfwGetCurrentContext ());
std::cout << " glGenVertexArrays " << VertexArrayID << std::endl;
}
        ready = true;
        glBindVertexArray (VertexArrayID);
        object->setupVertexAttributes ();
      }
    else
      {
        glBindVertexArray (VertexArrayID);
      }
  }

private:

  const T * object = nullptr;
  mutable bool ready = false;
  mutable GLuint VertexArrayID = 0;
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
