#pragma once

#include <memory>
#include <eccodes.h>

class glGribHandle
{
public:
  glGribHandle () 
  {
  }
  glGribHandle (codes_handle * _h)
  {
    h = _h;
  }
  ~glGribHandle () 
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

typedef std::shared_ptr<glGribHandle> glgrib_handle_ptr;


