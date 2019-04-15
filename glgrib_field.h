#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_coords_world.h"

class glgrib_field : public glgrib_world
{
public:
  void init (const char *, const glgrib_coords_world *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_GRADIENT_FLAT_SCALE;
    }
  virtual bool use_alpha () { return false; }
  bool hidden = false;
  void toggle_hide () { hidden = ! hidden; }
  virtual void render (const glgrib_view *) const;
};

#endif
