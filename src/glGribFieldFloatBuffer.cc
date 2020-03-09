#include "glGribFieldFloatBuffer.h"


glGrib::FieldFloatBuffer::FieldFloatBuffer (size_t size)
{
  data_ = new float[size];
}

glGrib::FieldFloatBuffer::FieldFloatBuffer (float * data)
{
  data_ = data;
}

glGrib::FieldFloatBuffer::~FieldFloatBuffer ()
{
  if (data_)
    delete [] data_;
  data_ = nullptr;
}

glGrib::FieldFloatBufferPtr glGrib::newGlgribFieldFloatBufferPtr (size_t size)
{
  return std::make_shared<glGrib::FieldFloatBuffer>(size);
}

glGrib::FieldFloatBufferPtr glGrib::newGlgribFieldFloatBufferPtr (float * data)
{
  return std::make_shared<glGrib::FieldFloatBuffer>(data);
}
