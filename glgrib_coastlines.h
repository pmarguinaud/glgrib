#ifndef _GLGRIB_COASTLINES_H
#define _GLGRIB_COASTLINES_H

#include "glgrib_polygon.h"

class glgrib_coastlines : public glgrib_polygon
{
public:
  void init (const char *);
  virtual int get_program_kind () const { return 2; }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view *) const;
};

#endif
