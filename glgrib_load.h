#ifndef _GLGRIB_LOAD_H
#define _GLGRIB_LOAD_H

#include <string>
#include "glgrib_field_metadata.h"

extern void glgrib_load (const std::vector<std::string> &, float, float **, glgrib_field_metadata *, int = 1, int = 0);
extern void glgrib_load (const std::string &, float **, glgrib_field_metadata *);

#endif
