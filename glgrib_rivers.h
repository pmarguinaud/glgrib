#ifndef _GLGRIB_RIVERS_H
#define _GLGRIB_RIVERS_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_rivers : public glgrib_lines
{
public:
  void setup (const glgrib_options_rivers &);
  const glgrib_options_rivers & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_rivers opts;
};


#endif
