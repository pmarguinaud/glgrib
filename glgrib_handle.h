#pragma once

#include <memory>
#include <eccodes.h>

class glgrib_handle
{
public:
  glgrib_handle () 
  {
  }
  glgrib_handle (codes_handle * _h)
  {
    h = _h;
  }
  ~glgrib_handle () 
  {
    if (h != NULL)
      codes_handle_delete (h);
  }
  codes_handle * getCodesHandle () 
  {
    return this == NULL ? NULL : h;
  }
private:
  codes_handle * h = NULL;
};

typedef std::shared_ptr<glgrib_handle> glgrib_handle_ptr;


