#ifndef _GLGRIB_GRID_H
#define _GLGRIB_GRID_H

#include "glgrib_polygon.h"

class glgrib_grid : public glgrib_polygon
{
public:
  void init ();
  virtual int get_program_kind () { return 2; }
  virtual bool use_alpha () { return false; }
};

#endif
