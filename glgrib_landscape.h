#ifndef _GLGRIB_LANDSCAPE_H
#define _GLGRIB_LANDSCAPE_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_opengl.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_landscape : public glgrib_world
{
public:
  glgrib_landscape & operator=(const glgrib_landscape &);
  virtual void init (const glgrib_options &, const glgrib_geometry_ptr);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  glgrib_opengl_texture_ptr texture;
  void render (const glgrib_view *) const;
  virtual ~glgrib_landscape ();
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
};

#endif
