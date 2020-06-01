#include "glGribFieldFloatBuffer.h"


glGrib::FieldFloatBuffer::FieldFloatBuffer (size_t size)
{
  if (size > 0)
    data_ = new float[size];
  size_ = size;
}

glGrib::FieldFloatBuffer::~FieldFloatBuffer ()
{
  if (data_)
    delete [] data_;
  data_ = nullptr;
  size_ = 0;
}

glGrib::FieldFloatBufferPtr glGrib::newGlgribFieldFloatBufferPtr (size_t size)
{
  return std::make_shared<glGrib::FieldFloatBuffer>(size);
}

