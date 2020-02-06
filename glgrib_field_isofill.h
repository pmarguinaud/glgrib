#ifndef _GLGRIB_FIELD_ISOFILL_H
#define _GLGRIB_FIELD_ISOFILL_H

#include "glgrib_field.h"

class glgrib_field_isofill : public glgrib_field
{
public:
  glgrib_field::kind getKind () const 
  {
    return glgrib_field::ISOFILL;
  }
  glgrib_field_isofill * clone () const;
  glgrib_field_isofill & operator= (const glgrib_field_isofill &);
  glgrib_field_isofill () { }
  glgrib_field_isofill (const glgrib_field_isofill &);
  virtual void setup (glgrib_loader *, const glgrib_options_field &, float = 0) override;
  virtual void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_field_isofill ();
  void setupVertexAttributes ();
  virtual void clear () override; 
  virtual bool useColorBar () const override { return true; }
  virtual int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:

  // Single color band
  class isoband_t
  {
  public:
    glgrib_option_color color;
    GLuint VertexArrayID = 0;
    glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
    int size;
  };

  struct
  {
    // Triangles with a single color
    GLuint VertexArrayID = 0;
    glgrib_opengl_buffer_ptr colorbuffer;

    // Isoband
    std::vector<isoband_t> isoband;
  } d;


};

#endif
