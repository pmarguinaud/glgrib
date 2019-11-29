#ifndef _GLGRIB_LOADER_H
#define _GLGRIB_LOADER_H

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"
#include "glgrib_handle.h"
#include "glgrib_geometry.h"
#include "glgrib_options.h"

class glgrib_loader 
{
public:
  static void uv2nd (const_glgrib_geometry_ptr,
                     const glgrib_field_float_buffer_ptr, 
                     const glgrib_field_float_buffer_ptr,
                     glgrib_field_float_buffer_ptr &, 
                     glgrib_field_float_buffer_ptr &,
                     const glgrib_field_metadata &, 
                     const glgrib_field_metadata &,
                     glgrib_field_metadata &, 
                     glgrib_field_metadata &);

  glgrib_handle_ptr handle_from_file (const std::string &);
  void load (glgrib_field_float_buffer_ptr *, const std::vector<std::string> &, const glgrib_options_geometry &, float, glgrib_field_metadata *, int = 1, int = 0, bool diff = false);
  void load (glgrib_field_float_buffer_ptr *, const std::string &, const glgrib_options_geometry &, glgrib_field_metadata *);
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


#endif
