#ifndef _GLGRIB_LANDSCAPE_TEX_H
#define _GLGRIB_LANDSCAPE_TEX_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_landscape_tex : public glgrib_world
{
public:
  virtual void init (const glgrib_options &, const glgrib_geometry &);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  GLuint textureID;
  void render (const glgrib_view *) const;
  virtual ~glgrib_landscape_tex ();
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
};

#endif
