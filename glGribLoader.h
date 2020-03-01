#pragma once

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glGribFieldMetadata.h"
#include "glGribFieldFloatBuffer.h"
#include "glGribHandle.h"
#include "glGribGeometry.h"
#include "glGribOptions.h"

class glGribLoader 
{
public:

  static void uv2nd (const_glgrib_geometry_ptr,
                     const glgrib_field_float_buffer_ptr, 
                     const glgrib_field_float_buffer_ptr,
                     glgrib_field_float_buffer_ptr &, 
                     glgrib_field_float_buffer_ptr &,
                     const glGribFieldMetadata &, 
                     const glGribFieldMetadata &,
                     glGribFieldMetadata &, 
                     glGribFieldMetadata &);

  glgrib_handle_ptr handle_from_file (const std::string &);
  void load (glgrib_field_float_buffer_ptr *, const std::vector<std::string> &, const glGribOptionsGeometry &, float, glGribFieldMetadata *, int = 1, int = 0, bool diff = false);
  void load (glgrib_field_float_buffer_ptr *, const std::string &, const glGribOptionsGeometry &, glGribFieldMetadata *);
  void setSize (int _size) { size = _size; }
private:
  int size = 0;

  class entry_t
  {
  public:
    std::string file;
    glgrib_handle_ptr ghp;
  };

  typedef std::list<entry_t> cache_t;
  cache_t cache;
};
