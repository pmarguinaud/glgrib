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
  void setup (const glgrib_options_lines &, const std::vector<unsigned int> & = {0x00000000}, 
             const std::vector<unsigned int> & = {0x00000000});
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  void resize (const glgrib_view &) override {}
  const glgrib_options_lines & getOptions () const { return opts; }
private:
  glgrib_options_lines opts;
};

#endif
