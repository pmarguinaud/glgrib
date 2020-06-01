#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>


namespace glGrib
{

template <typename T>
class OpenGLBuffer
{
private:

  class OpenGLBufferMapping
  {
  public:
    OpenGLBufferMapping (OpenGLBuffer * b) : buffer (b) 
    {
      ptr = (T *)glMapNamedBuffer (buffer->id (), GL_READ_WRITE);
    }
    ~OpenGLBufferMapping ()
    {
      clear ();
    }
    template <typename I>
    T & operator [] (I i) 
    {
      return ptr[i];
    }
    void clear ()
    {
      glUnmapNamedBuffer (buffer->id ());
      ptr = nullptr;
    }
    T * address ()
    {
      return ptr;
    }
  private:
    OpenGLBuffer * buffer;
    T * ptr = nullptr;
  };

public:

  OpenGLBuffer (size_t size, const T * data = nullptr)
  {
    glGenBuffers (1, &id_);
    
//glNamedBufferData (id_, size, data, GL_STATIC_DRAW); does not work
//glNamedBufferStorage (id_, size, data, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT); does not work

    glBindBuffer (GL_ARRAY_BUFFER, id_);
    glBufferData (GL_ARRAY_BUFFER, sizeof (T) * size, data, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    allocated_ = true;
    size_ = size;
  }
  ~OpenGLBuffer ()
  {
    if (allocated_)
      glDeleteBuffers (1, &id_);
    allocated_ = false;
  }

  GLuint id () { return id_; }
  bool allocated () const { return allocated_; }
  void bind (GLenum target, GLuint index = 0) const
  {
    if (index == 0)
      glBindBuffer (target, id_);
    else
      glBindBufferBase (target, index, id_);
  }
  size_t buffersize () const { return size_; }

  OpenGLBufferMapping map ()
  {
    return OpenGLBufferMapping (this);
  }

private:
  bool allocated_ = false;
  GLuint id_;
  size_t size_;
};

template <typename T>
class OpenGLBufferPtr : public std::shared_ptr<OpenGLBuffer<T>>
{
public:
  OpenGLBufferPtr () = default;
  OpenGLBufferPtr (size_t size, const T * data = nullptr)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(size, data))
  {
  }
  OpenGLBufferPtr (const std::vector<T> & v)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(v.size (), v.data ()))
  {
  }
};

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

class OpenGLTexturePtr : public std::shared_ptr<OpenGLTexture>
{
public:
  OpenGLTexturePtr () = default;
  OpenGLTexturePtr (int width, int height, const void * data)
    : std::shared_ptr<OpenGLTexture> (new OpenGLTexture (width, height, data))
  {
  }
};

template <typename T>
class OpenGLVertexArray
{
private:

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

  void render () const
  {
    bind ();
    object->render ();
    unbind ();
  }

  template <typename T0>
  void render (const T0 & a0) const
  {
    bind ();
    object->render (a0);
    unbind ();
  }

  template <typename T0, typename T1>
  void render (const T0 & a0, const T1 & a1) const
  {
    bind ();
    object->render (a0, a1);
    unbind ();
  }

  template <typename T0, typename T1, typename T2>
  void render (const T0 & a0, const T1 & a1, const T2 & a2) const
  {
    bind ();
    object->render (a0, a1, a2);
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
