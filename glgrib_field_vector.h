#ifndef _GLGRIB_FIELD_VECTOR_H
#define _GLGRIB_FIELD_VECTOR_H

#include "glgrib_field.h"

class glgrib_field_vector : public glgrib_field
{
public:
  glgrib_field_vector * clone () const;
  glgrib_field_vector & operator= (const glgrib_field_vector &);
  glgrib_field_vector () { }
  glgrib_field_vector (const glgrib_field_vector &);
  virtual void init (const glgrib_options_field &, int = 0);
  virtual glgrib_program::kind get_program_kind () const 
    { 
      return glgrib_program::GRADIENT_FLAT_SCALE_VECTOR;
    }
  virtual void render (const glgrib_view *) const;
  virtual ~glgrib_field_vector ();
  void setupVertexAttributes ();
private:
  glgrib_opengl_buffer_ptr buffer_n, buffer_d;
  GLuint VertexArrayIDvector = 0;
protected:
  virtual void cleanup ();
};

#endif
