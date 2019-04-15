#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"
#include "glgrib_coords_world.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void init (const char *, const glgrib_coords_world *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return flat ? GLGRIB_PROGRAM_RGB_FLAT : GLGRIB_PROGRAM_RGB; 
    }
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
  void def_from_vertexbuffer_col_elementbuffer 
          (const glgrib_coords_world *, unsigned char *);
};

#endif
