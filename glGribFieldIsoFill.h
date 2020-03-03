#pragma once

#include "glGribField.h"

class glGribFieldIsoFill : public glGribField
{
public:
  glGribField::kind getKind () const 
  {
    return glGribField::ISOFILL;
  }
  glGribFieldIsoFill * clone () const;
  glGribFieldIsoFill & operator= (const glGribFieldIsoFill &);
  glGribFieldIsoFill () { }
  glGribFieldIsoFill (const glGribFieldIsoFill &);
  void setup (glGribLoader *, const glGribOptionsField &, float = 0) override;
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribFieldIsoFill ();
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
    glGribOptionColor color;
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

