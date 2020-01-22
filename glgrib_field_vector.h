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
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_field_vector ();
  void setupVertexAttributes ();
  void reSample (const glgrib_view &);
  void toggleShowVector () { opts.vector.arrow.on = ! opts.vector.arrow.on; }
  void toggleShowNorm () { opts.vector.norm.on = ! opts.vector.norm.on; }
  virtual void resize (const glgrib_view &);
  virtual float getNormedMinValue () const
  {
    std::vector<float> val = getMinValue ();
    return val[0];
  }
  virtual float getNormedMaxValue () const
  {
    std::vector<float> val = getMaxValue ();
    return val[0];
  }
  virtual bool useColorBar () const { return true; }
  virtual int getSlotMax () const  
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
  virtual void clear ();
};

#endif
