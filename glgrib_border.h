#ifndef _GLGRIB_BORDER_H
#define _GLGRIB_BORDER_H

#include "glgrib_options.h"
#include "glgrib_lines.h"

class glgrib_border : public glgrib_lines
{
public:
  void setup (const glgrib_options_border &);
  const glgrib_options_border & getOptions () const { return opts; }
  virtual float getScale () const { return opts.lines.scale; }
private:
  glgrib_options_border opts;
};


#endif
