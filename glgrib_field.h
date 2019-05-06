#ifndef _GLGRIB_FIELD_H
#define _GLGRIB_FIELD_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"
#include "glgrib_palette.h"

#include <string>

class glgrib_field_display_options
{
public:
  float scale = 1.0;
  glgrib_palette palette;
  glgrib_field_display_options ();
};

class glgrib_field : public glgrib_world
{
public:
  void init (const std::string & field, const glgrib_options &, const glgrib_geometry_ptr);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_GRADIENT_FLAT_SCALE;
    }
  virtual bool use_alpha () { return false; }
  virtual void render (const glgrib_view * view) const { render (view, glgrib_field_display_options ()); }
  virtual void render (const glgrib_view *, const glgrib_field_display_options &) const;
  float * values = NULL;
  virtual float getValue (int index) const { return values[index]; }
  float valmis, valmin, valmax;
  virtual ~glgrib_field ();
};

#endif
