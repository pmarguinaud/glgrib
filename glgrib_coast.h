#ifndef _GLGRIB_COAST_H
#define _GLGRIB_COAST_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_coast : public glgrib_lines
{
public:
  void init (const glgrib_options_coast &);
  glgrib_options_coast opts;
};


#endif
