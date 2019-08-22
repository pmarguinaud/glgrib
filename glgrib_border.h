#ifndef _GLGRIB_BORDER_H
#define _GLGRIB_BORDER_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_border : public glgrib_lines
{
public:
  void init (const glgrib_options_border &);
  glgrib_options_border opts;
};


#endif
