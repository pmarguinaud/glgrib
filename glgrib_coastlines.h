#ifndef _GLGRIB_COASTLINES_H
#define _GLGRIB_COASTLINES_H

#include "glgrib_polygon.h"
#include <string>

class glgrib_coastlines : public glgrib_polygon
{
public:
  void init (const std::string &);
  virtual glgrib_program_kind get_program_kind () const { return GLGRIB_PROGRAM_MONO; }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
};

#endif
