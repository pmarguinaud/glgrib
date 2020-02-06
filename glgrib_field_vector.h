#ifndef _GLGRIB_FIELD_VECTOR_H
#define _GLGRIB_FIELD_VECTOR_H

#include "glgrib_field.h"
#include "glgrib_view.h"

class glgrib_field_vector : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::VECTOR;
  }
  glgrib_field_vector * clone () const;
  glgrib_field_vector & operator= (const glgrib_field_vector &);
  glgrib_field_vector () { }
  glgrib_field_vector (const glgrib_field_vector &);
  void setup (glgrib_loader *, const glgrib_options_field &, float = 0) override;
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_field_vector ();
  void setupVertexAttributes ();
  void reSample (const glgrib_view &);
  void toggleShowVector () { opts.vector.arrow.on = ! opts.vector.arrow.on; }
  void toggleShowNorm () { opts.vector.norm.on = ! opts.vector.norm.on; }
  void resize (const glgrib_view &) override;
  float getNormedMinValue () const override
  {
    std::vector<float> val = getMinValue ();
    return val[0];
  }
  float getNormedMaxValue () const override
  {
    std::vector<float> val = getMaxValue ();
    return val[0];
  }
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size () / 2;
  }
private:
  GLuint VertexArrayIDvector = 0;
  struct
    {
      glgrib_opengl_buffer_ptr buffer_n, buffer_d;
      float vscale;
    } d;
protected:
  void clear () override;
};

#endif
