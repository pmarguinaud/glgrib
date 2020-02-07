#pragma once

#include <memory>

class glgrib_field_float_buffer
{
public:
  glgrib_field_float_buffer (size_t);
  glgrib_field_float_buffer (float *);
  ~glgrib_field_float_buffer ();
  float * data () { return data_; }
  float & operator[] (int i) { return data_[i]; }
private:
  float * data_ = NULL;
};

typedef std::shared_ptr<glgrib_field_float_buffer> glgrib_field_float_buffer_ptr;

extern glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (size_t);
extern glgrib_field_float_buffer_ptr new_glgrib_field_float_buffer_ptr (float *);

