#ifndef _GLGRIB_FIELD_SCALAR_H
#define _GLGRIB_FIELD_SCALAR_H

#include "glgrib_field.h"

class glgrib_field_scalar : public glgrib_field
{
public:
  glgrib_field_scalar * clone () const;
  glgrib_field_scalar & operator= (const glgrib_field_scalar &);
  glgrib_field_scalar () { }
  glgrib_field_scalar (const glgrib_field_scalar &);
  virtual void init (const glgrib_options_field &, int = 0);
  virtual glgrib_program::kind get_program_kind () const 
    { 
      return glgrib_program::GRADIENT_FLAT_SCALE_SCALAR;
    }
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_scalar ();
  void setupVertexAttributes ();
};

#endif
