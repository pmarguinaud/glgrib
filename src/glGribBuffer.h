#pragma once

#include <memory>
#include <stdexcept>

namespace glGrib
{

template <typename T>
class Buffer
{
public:
  template <typename I>
  const T & operator[] (I i) const
  { 
#ifdef GLGRIB_CHECK_BOUNDS
      if (((i >= 0) && (static_cast<size_t>(i) >= size_)) || (i < 0))
        throw std::runtime_error ("Out of bounds access");
#endif
    return data_[i]; 
  }
  template <typename I>
  T & operator[] (I i) 
  { 
#ifdef GLGRIB_CHECK_BOUNDS
      if (((i >= 0) && (static_cast<size_t>(i) >= size_)) || (i < 0))
        throw std::runtime_error ("Out of bounds access");
#endif
    return data_[i]; 
  }
  Buffer () = default;
  explicit Buffer (size_t size)
  {
    if (size > 0)
      data_ = new T[size];
    size_ = size;
  }
  ~Buffer ()
  {
    if (data_)
      delete [] data_;
    data_ = nullptr;
    size_ = 0;
  }
  bool allocated () const
  {
    return data_ != nullptr;
  }
  size_t size () const
  {
    return size_;
  }
  Buffer & operator= (const Buffer &) = delete;
  Buffer (const Buffer &) = delete;
  void allocate (size_t size)
  {
    if (data_)
      delete [] data_;
    if (size > 0)
      data_ = new T[size];
    size_ = size;
  }
private:
  T * data_ = nullptr;
  size_t size_ = 0;
};

template <typename T>
class BufferPtr : public std::shared_ptr<Buffer<T>>
{
public:
  BufferPtr () = default;
  explicit BufferPtr (size_t size)
    : std::shared_ptr<Buffer<T>> (new Buffer<T> (size))
  {
  }
  template <typename I>
  T & operator[] (I i) 
  { 
    Buffer<T> * b = this->get ();
    return (*b)[i]; 
  }
  template <typename I>
  const T & operator[] (I i) const 
  { 
    const Buffer<T> * b = this->get ();
    return (*b)[i];
  }
  bool allocated () const
  {
    const Buffer<T> * b = this->get ();
    if (b == nullptr)
      return false;
    return b->allocated ();
  }
};

}
