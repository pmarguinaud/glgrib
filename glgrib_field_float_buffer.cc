#include "glgrib_field_float_buffer.h"


glgrib_field_float_buffer::glgrib_field_float_buffer (size_t size)
{
  data_ = new float[size];
}

glgrib_field_float_buffer::glgrib_field_float_buffer (float * data)
{
  data_ = data;
}

glgrib_field_float_buffer::~glgrib_field_float_buffer ()
{
  if (data_)
    delete [] data_;
  data_ = nullptr;
}

glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (size_t size)
{
  return std::make_shared<glgrib_field_float_buffer>(size);
}

glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (float * data)
{
  return std::make_shared<glgrib_field_float_buffer>(data);
}
