#pragma once

#include "glGribField.h"

namespace glGrib
{

class FieldIsoFill : public Field
{
public:
  Field::kind getKind () const 
  {
    return Field::ISOFILL;
  }
  FieldIsoFill * clone () const;
  FieldIsoFill & operator= (const FieldIsoFill &);
  FieldIsoFill () { }
  FieldIsoFill (const FieldIsoFill &);
  void setup (Loader *, const OptionsField &, float = 0) override;
  void render (const View &, const OptionsLight &) const override;
  virtual ~FieldIsoFill ();
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
    OptionColor color;
    GLuint VertexArrayID = 0;
    OpenGLBufferPtr vertexbuffer, elementbuffer;
    int size;
  };

  struct
  {
    // Triangles with a single color
    GLuint VertexArrayID = 0;
    OpenGLBufferPtr colorbuffer;

    // Isoband
    std::vector<isoband_t> isoband;
  } d;


};


}
