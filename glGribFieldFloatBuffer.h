#pragma once

#include <memory>

class glGribFieldFloatBuffer
{
public:
  glGribFieldFloatBuffer (size_t);
  glGribFieldFloatBuffer (float *);
  ~glGribFieldFloatBuffer ();
  float * data () { return data_; }
  float & operator[] (int i) { return data_[i]; }
private:
  float * data_ = nullptr;
};

typedef std::shared_ptr<glGribFieldFloatBuffer> glGribFieldFloatBufferPtr;

extern glGribFieldFloatBufferPtr newGlgribFieldFloatBufferPtr (size_t);
extern glGribFieldFloatBufferPtr newGlgribFieldFloatBufferPtr (float *);

