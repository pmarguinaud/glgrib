#ifndef _GLGRIB_LANDSCAPE_H
#define _GLGRIB_LANDSCAPE_H

#include "glgrib_world.h"
#include "glgrib_coords_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_landscape : public glgrib_world
{
public:
  virtual void init (const glgrib_options &, const glgrib_coords_world *, const glgrib_geometry &) = 0;
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
