#ifndef _GLGRIB_LOADER_H
#define _GLGRIB_LOADER_H

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"
#include "glgrib_handle.h"

class glgrib_loader 
{
public:
  glgrib_handle_ptr handle_from_file (const std::string &);
  glgrib_field_float_buffer_ptr load (const std::vector<std::string> &, float, glgrib_field_metadata *, 
                                      int = 1, int = 0, bool diff = false);
  glgrib_field_float_buffer_ptr load (const std::string &, glgrib_field_metadata *);
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
