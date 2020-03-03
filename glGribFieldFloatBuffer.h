#pragma once

#include <memory>

namespace glGrib
{

class FieldFloatBuffer
{
public:
  FieldFloatBuffer (size_t);
  FieldFloatBuffer (float *);
  ~FieldFloatBuffer ();
  float * data () { return data_; }
  float & operator[] (int i) { return data_[i]; }
private:
  float * data_ = nullptr;
};

typedef std::shared_ptr<FieldFloatBuffer> FieldFloatBufferPtr;

extern FieldFloatBufferPtr newGlgribFieldFloatBufferPtr (size_t);
extern FieldFloatBufferPtr newGlgribFieldFloatBufferPtr (float *);


}
