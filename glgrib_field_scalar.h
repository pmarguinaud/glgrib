#ifndef _GLGRIB_FIELD_SCALAR_H
#define _GLGRIB_FIELD_SCALAR_H

#include "glgrib_field.h"

class glgrib_field_scalar : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::SCALAR;
  }
  glgrib_field_scalar * clone () const;
  glgrib_field_scalar & operator= (const glgrib_field_scalar &);
  glgrib_field_scalar () { }
  glgrib_field_scalar (const glgrib_field_scalar &);
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_scalar ();
  void setupVertexAttributes ();
  virtual bool useColorBar () const { return true; }
private:
  GLuint VertexArrayIDpoints = 0;
  virtual void clear ();
};

#endif
