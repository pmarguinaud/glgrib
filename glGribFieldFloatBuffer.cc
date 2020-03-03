#include "glGribFieldFloatBuffer.h"


glGribFieldFloatBuffer::glGribFieldFloatBuffer (size_t size)
{
  data_ = new float[size];
}

glGribFieldFloatBuffer::glGribFieldFloatBuffer (float * data)
{
  data_ = data;
}

glGribFieldFloatBuffer::~glGribFieldFloatBuffer ()
{
  if (data_)
    delete [] data_;
  data_ = nullptr;
}

glGribFieldFloatBufferPtr newGlgribFieldFloatBufferPtr (size_t size)
{
  return std::make_shared<glGribFieldFloatBuffer>(size);
}

glGribFieldFloatBufferPtr newGlgribFieldFloatBufferPtr (float * data)
{
  return std::make_shared<glGribFieldFloatBuffer>(data);
}
