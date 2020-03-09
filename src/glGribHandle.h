#pragma once

#include <memory>
#include <eccodes.h>

namespace glGrib
{

class Handle
{
public:
  Handle () 
  {
  }
  Handle (codes_handle * _h)
  {
    h = _h;
  }
  ~Handle () 
  {
    if (h != nullptr)
      codes_handle_delete (h);
  }
  codes_handle * getCodesHandle () 
  {
    return this == nullptr ? nullptr : h;
  }
private:
  codes_handle * h = nullptr;
};

typedef std::shared_ptr<Handle> HandlePtr;



}
