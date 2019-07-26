#ifndef _GLGRIB_FIELD_VECTOR_H
#define _GLGRIB_FIELD_VECTOR_H

#include "glgrib_field.h"
#include "glgrib_view.h"

class glgrib_field_vector : public glgrib_field
{
public:
  glgrib_field_vector * clone () const;
  glgrib_field_vector & operator= (const glgrib_field_vector &);
  glgrib_field_vector () { }
  glgrib_field_vector (const glgrib_field_vector &);
  virtual void init (glgrib_loader *, const glgrib_options_field &, float = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_vector ();
  void setupVertexAttributes ();
  void reSample (const glgrib_view &);
  void toggleShowVector () { opts.vector.hide_arrow = ! opts.vector.hide_arrow; }
  void toggleShowNorm () { opts.vector.hide_norm = ! opts.vector.hide_norm; }
  virtual void resize (const glgrib_view &);
private:
  GLuint VertexArrayIDvector = 0;
  struct
    {
      glgrib_opengl_buffer_ptr buffer_n, buffer_d;
      float vscale;
    } d;
protected:
  virtual void cleanup ();
};

#endif
