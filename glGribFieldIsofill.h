#pragma once

#include "glGribField.h"

class glGribFieldIsofill : public glGribField
{
public:
  glGribField::kind getKind () const 
  {
    return glGribField::ISOFILL;
  }
  glGribFieldIsofill * clone () const;
  glGribFieldIsofill & operator= (const glGribFieldIsofill &);
  glGribFieldIsofill () { }
  glGribFieldIsofill (const glGribFieldIsofill &);
  void setup (glGribLoader *, const glGribOptionsField &, float = 0) override;
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribFieldIsofill ();
  void setupVertexAttributes ();
  void clear () override; 
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
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

