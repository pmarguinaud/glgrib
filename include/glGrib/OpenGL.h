#pragma once

#ifdef GLGRIB_USE_EGL

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#ifdef GLGRIB_USE_GBM
#include <gbm.h>
#endif

#endif

#ifdef GLGRIB_USE_GLFW

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif

#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

#include "glGrib/Buffer.h"
#include "glGrib/Options.h"


namespace glGrib
{

#ifdef GLGRIB_USE_EGL
class eglDisplay
{
public:
#ifdef GLGRIB_USE_GBM
  eglDisplay (const std::string &);
#else
  eglDisplay (int);
#endif
  ~eglDisplay ();
  EGLDisplay display = nullptr;
  EGLConfig  config  = nullptr;
#ifdef GLGRIB_USE_GBM
  int fd = -1;
  struct gbm_device * gbm = nullptr;
#endif
private:
  void setup ();
};

bool preEGLError ();

extern eglDisplay * egl;
#endif

typedef struct
{
  int minor;
  int major;
} OpenGLVersion;

const OpenGLVersion getOpenGLVersion (float);

template <typename T>
class OpenGLBuffer
{
public:

  class Mapping
  {
  public:
    // Avoid copies
    Mapping & operator= (Mapping &) = delete;
    Mapping (const Mapping &) = delete;
    // constructor elision, but the move constructor is required anyway
    Mapping (Mapping && other)
    {
      std::swap (ptr, other.ptr);
      std::swap (buffer, other.buffer);
    }
    explicit Mapping (OpenGLBuffer * b) : buffer (b) 
    {
#ifdef GLGRIB_USE_EGL
      static PFNGLMAPNAMEDBUFFERPROC glMapNamedBuffer = nullptr;
      if (glMapNamedBuffer == nullptr)
        glMapNamedBuffer = (PFNGLMAPNAMEDBUFFERPROC)eglGetProcAddress ("glMapNamedBuffer");
#endif
      ptr = (T *)glMapNamedBuffer (buffer->id (), GL_READ_WRITE);
    }
    ~Mapping ()
    {
      clear ();
    }
    template <typename I>
    T & operator [] (I i) 
    {
#ifdef GLGRIB_CHECK_BOUNDS
      if (((i > 0) && (static_cast<size_t> (i) >= buffer->size ())) || (i < 0))
        throw std::runtime_error ("Out of bounds access");
#endif
      return ptr[i];
    }

    size_t size () const 
    {
      return buffer->size ();
    }

  private:
    void clear ()
    {
      if (ptr)
        {
#ifdef GLGRIB_USE_EGL
          static PFNGLUNMAPNAMEDBUFFERPROC glUnmapNamedBuffer = nullptr;
          if (glUnmapNamedBuffer == nullptr)
            glUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFERPROC)eglGetProcAddress ("glUnmapNamedBuffer");
#endif
          glUnmapNamedBuffer (buffer->id ());
          ptr = nullptr;
        }
    }
  private:
    OpenGLBuffer * buffer;
    T * ptr = nullptr;
  };


  explicit OpenGLBuffer (const Buffer<T> & b)
  {
    glGenBuffers (1, &id_);
    
    glBindBuffer (GL_ARRAY_BUFFER, id_);
    glBufferData (GL_ARRAY_BUFFER, sizeof (T) * b.size (), &b[0], GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    allocated_ = true;
    size_ = b.size ();
  }

  explicit OpenGLBuffer (const BufferPtr<T> & b)
  {
    glGenBuffers (1, &id_);
    
    glBindBuffer (GL_ARRAY_BUFFER, id_);
    glBufferData (GL_ARRAY_BUFFER, sizeof (T) * b->size (), &b[0], GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    allocated_ = true;
    size_ = b->size ();
  }

  explicit OpenGLBuffer (size_t size, const T * data = nullptr)
  {
    glGenBuffers (1, &id_);
    
if (0) {
    // Need to bind the buffer to activate it
    glBindBuffer (GL_ARRAY_BUFFER, id_);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glNamedBufferData (id_, size, data, GL_STATIC_DRAW); 
}else{
    glBindBuffer (GL_ARRAY_BUFFER, id_);
    glBufferData (GL_ARRAY_BUFFER, sizeof (T) * size, data, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
}

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

  size_t size () const { return size_; }

  Mapping map ()
  {
    return Mapping (this);
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
  explicit OpenGLBufferPtr (size_t size, const T * data = nullptr)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(size, data)) { }
  explicit OpenGLBufferPtr (const std::vector<T> & v)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(v.size (), v.data ())) { }
  explicit OpenGLBufferPtr (const BufferPtr<T> & b)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(b)) { }
  explicit OpenGLBufferPtr (const Buffer<T> & b)
  : std::shared_ptr<OpenGLBuffer<T>> (new OpenGLBuffer<T>(b)) { }
};

class OpenGLTexture
{
public:
  explicit OpenGLTexture (int width, int height, const BufferPtr<unsigned char> & data, 
                          GLint internalformat = GL_RGB, GLint format = GL_RGB)
  {
    int t = 0;

    if (format == GL_RGB)
      t = 3;
    else if (format == GL_RGBA)
      t = 4;

    assert ((width * height * t) == static_cast<int> (data->size ()));

    init (width, height, &data[0], internalformat, format);
  }
  explicit OpenGLTexture (int width, int height, const void * data, 
                          GLint internalformat = GL_RGB, GLint format = GL_RGB)
  {
    init (width, height, data, internalformat, format);
  }
  ~OpenGLTexture ();
  GLuint id () { return id_; }
  bool allocated () { return allocated_; }
  void bind (GLuint target)
  {
    if (target != 0)
      throw std::runtime_error ("Unexpected texture target");
    glActiveTexture (GL_TEXTURE0); 
    glBindTexture (GL_TEXTURE_2D, id_);
  }
private:
  void init (int, int, const void *, GLint, GLint);
  bool allocated_ = false;
  GLuint id_;
};

class OpenGLTexturePtr : public std::shared_ptr<OpenGLTexture>
{
public:
  OpenGLTexturePtr () = default;
  explicit OpenGLTexturePtr (int width, int height, const BufferPtr<unsigned char> & data,
                             GLint internalformat = GL_RGB, GLint format = GL_RGB)
    : std::shared_ptr<OpenGLTexture> (new OpenGLTexture (width, height, 
                                      data, internalformat, format)) { }
  explicit OpenGLTexturePtr (int width, int height, const void * data, 
                             GLint internalformat = GL_RGB, GLint format = GL_RGB)
    : std::shared_ptr<OpenGLTexture> (new OpenGLTexture (width, height, 
                                      data, internalformat, format)) { }
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

  explicit OpenGLVertexArray (T * _object) : object (_object) {}

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
      glDeleteVertexArrays (1, &VertexArrayID);
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

void glStart (const OptionsRender &);
void glStop ();
void glSetupDebug (const OptionsRender & opts);

template <typename T> GLenum getOpenGLType ();
template <> GLenum getOpenGLType<unsigned char > ();
template <> GLenum getOpenGLType<unsigned short> ();
template <> GLenum getOpenGLType<unsigned int  > ();
template <> GLenum getOpenGLType<float         > ();

namespace OpenGL
{
  const unsigned long int restart = 0xffffffff;
};


}
