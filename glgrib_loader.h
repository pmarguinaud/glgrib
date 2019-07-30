#ifndef _GLGRIB_LOAD_H
#define _GLGRIB_LOAD_H

#include <string>
#include <list>
#include <eccodes.h>
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"


class glgrib_loader 
{
public:
  glgrib_field_float_buffer_ptr load (const std::vector<std::string> &, float, glgrib_field_metadata *, 
                                      int = 1, int = 0, bool diff = false);
  glgrib_field_float_buffer_ptr load (const std::string &, glgrib_field_metadata *);
  void setSize (int _size) { size = _size; }
private:
  int size = 0;
  class entry
  {
  public:
    std::string file;
    glgrib_field_float_buffer_ptr data;
    glgrib_field_metadata meta;
  };
  std::list<entry> cache;
};

extern codes_handle * glgrib_handle_from_file (const std::string &);

#endif
