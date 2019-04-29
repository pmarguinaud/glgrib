#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_coords_world.h"
#include "glgrib_options.h"

#include <string>

class glgrib_field : public glgrib_world
{
public:
  void init (const glgrib_options &, const glgrib_coords_world *);
  glgrib_options opts;
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
