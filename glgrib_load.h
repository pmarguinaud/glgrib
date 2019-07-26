#ifndef _GLGRIB_LOAD_H
#define _GLGRIB_LOAD_H

#include <string>
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"

extern glgrib_field_float_buffer_ptr glgrib_load (const std::vector<std::string> &, float, glgrib_field_metadata *, int = 1, int = 0);
extern glgrib_field_float_buffer_ptr glgrib_load (const std::string &, glgrib_field_metadata *);

#endif
