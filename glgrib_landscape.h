#ifndef _GLGRIB_LANDSCAPE_H
#define _GLGRIB_LANDSCAPE_H

#include "glgrib_world.h"
#include "glgrib_coords_world.h"

class glgrib_landscape : public glgrib_world
{
public:
  virtual void init (const char *, const glgrib_coords_world *) = 0;
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return flat ? GLGRIB_PROGRAM_RGB_FLAT : GLGRIB_PROGRAM_RGB; 
    }
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
};

#endif
