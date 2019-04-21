#ifndef _GLGRIB_LANDSCAPE_TEX_H
#define _GLGRIB_LANDSCAPE_TEX_H

#include "glgrib_world.h"
#include "glgrib_coords_world.h"

class glgrib_landscape_tex : public glgrib_world
{
public:
  void init (const char *, const glgrib_coords_world *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; }
  GLuint textureID;
  void render (const glgrib_view *) const;
  ~glgrib_landscape_tex ();
};

#endif
