#ifndef _GLGRIB_COASTLINES_H
#define _GLGRIB_COASTLINES_H

#include "glgrib_polygon.h"
#include "glgrib_options.h"
#include <string>

class glgrib_coastlines : public glgrib_polygon
{
public:
  glgrib_coastlines & operator=(const glgrib_coastlines &);
  void init (const glgrib_options_coastlines &);
  virtual glgrib_program::kind get_program_kind () const { return glgrib_program::MONO; }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
  glgrib_options_coastlines opts;
};

#endif
