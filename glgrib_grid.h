#ifndef _GLGRIB_GRID_H
#define _GLGRIB_GRID_H

#include "glgrib_polygon.h"
#include "glgrib_options.h"

class glgrib_grid : public glgrib_polygon
{
public:
  glgrib_grid & operator=(const glgrib_grid &);
  void setup (const glgrib_options_grid &);
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual void resize (const glgrib_view &) {}
  const glgrib_options_grid & getOptions () const { return opts; }
  void setColorOptions (const glgrib_option_color & o)
  {
    opts.color = o;
  }
  void setScaleOptions (float s)
  {
    opts.scale = s;
  }
private:
  glgrib_options_grid opts;
};

#endif
