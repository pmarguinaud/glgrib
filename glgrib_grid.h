#ifndef _GLGRIB_GRID_H
#define _GLGRIB_GRID_H

#include "glgrib_polygon.h"
#include "glgrib_options.h"

class glgrib_grid : public glgrib_polygon
{
public:
  glgrib_grid & operator=(const glgrib_grid &);
  void init (const glgrib_options_grid &);
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  glgrib_options_grid opts;
  virtual void resize (const glgrib_view &) {}
};

#endif
