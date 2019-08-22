#ifndef _GLGRIB_RIVERS_H
#define _GLGRIB_RIVERS_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_rivers : public glgrib_lines
{
public:
  void init (const glgrib_options_rivers &);
  glgrib_options_rivers opts;
};


#endif
