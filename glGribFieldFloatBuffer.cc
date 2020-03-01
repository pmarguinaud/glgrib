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

glgrib_field_float_buffer_ptr newGlgribFieldFloatBufferPtr (size_t size)
{
  return std::make_shared<glGribFieldFloatBuffer>(size);
}

glgrib_field_float_buffer_ptr newGlgribFieldFloatBufferPtr (float * data)
{
  return std::make_shared<glGribFieldFloatBuffer>(data);
}
