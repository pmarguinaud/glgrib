#ifndef _GLGRIB_COAST_H
#define _GLGRIB_COAST_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_coast : public glgrib_lines
{
public:
  void setup (const glgrib_options_coast &);
  const glgrib_options_coast & getOptions () const { return opts; }
  float getScale () const override { return opts.lines.scale; }
private:
  glgrib_options_coast opts;
};


#endif
