#ifndef _GLGRIB_LINES_H
#define _GLGRIB_LINES_H

#include "glgrib_polygon.h"
#include "glgrib_options.h"
#include <string>
#include <vector>

class glgrib_lines : public glgrib_polygon
{
public:
  glgrib_lines & operator=(const glgrib_lines &);
  void init (const glgrib_options_lines &, const std::vector<unsigned int> & = {0x00000000}, 
             const std::vector<unsigned int> & = {0x00000000});
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  glgrib_options_lines opts;
  virtual void resize (const glgrib_view &) {}
};

#endif
