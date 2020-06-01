#pragma once

#include <memory>

namespace glGrib
{

class FieldFloatBuffer
{
public:
  FieldFloatBuffer (size_t);
  ~FieldFloatBuffer ();
  float * data () { return data_; }
  float & operator[] (int i) { return data_[i]; }
private:
  float * data_ = nullptr;
  size_t size_ = 0;
};

class FieldFloatBufferPtr : public std::shared_ptr<FieldFloatBuffer>
{
public:
  FieldFloatBufferPtr () = default;
  FieldFloatBufferPtr (size_t size)
    : std::shared_ptr<FieldFloatBuffer> (new FieldFloatBuffer (size))
  {
  }
  float & operator[] (int i) { return (*get ())[i]; }
  const float & operator[] (int i) const { return (*get ())[i]; }
};

}
