#pragma once

#include "glGribField.h"

namespace glGrib
{

class FieldIsoFill : public FieldPacked<8>
{
public:
  Field::kind getKind () const 
  {
    return Field::ISOFILL;
  }
  FieldIsoFill * clone () const;
  FieldIsoFill () : d (this) { }
  void setup (Loader *, const OptionsField &, float = 0) override;
  void render (const View &, const OptionsLight &) const override;
  void setupVertexAttributes () const;
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
    isoband_t () : VAID (this) {}
    isoband_t (const isoband_t & iso) : VAID (this)
    {
      d = iso.d;
    }
    void setupVertexAttributes () const;
    void clear ()
    {
      VAID.clear ();
    }
    void render () const;
    struct
    {
      OptionColor color;
      OpenGLBufferPtr<float> vertexbuffer;
      OpenGLBufferPtr<unsigned int> elementbuffer;
      int size;
    } d;
    OpenGLVertexArray<isoband_t> VAID;
  };

  class _d
  {
  public:
    _d (FieldIsoFill * f) : VAID (f) {}
    // Triangles with a single color
    OpenGLVertexArray<FieldIsoFill> VAID;
    OpenGLBufferPtr<T> colorbuffer;

    // Isoband
    std::vector<isoband_t> isoband;
  };
  _d d;

};


}
