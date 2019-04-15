#ifndef _GLGRIB_WORLD_H
#define _GLGRIB_WORLD_H

#include "glgrib_polyhedron.h"

class glgrib_world : public glgrib_polyhedron
{
public:
  void init (const char *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return flat ? GLGRIB_PROGRAM_RGB : GLGRIB_PROGRAM_RGB_FLAT; 
    }
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
};

#endif
