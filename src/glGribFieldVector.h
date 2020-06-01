#pragma once

#include "glGribField.h"
#include "glGribView.h"

namespace glGrib
{

class FieldVector : public FieldPacked<8>
{
public:
  using T = typename FieldPackingType<8>::type;
  
  Field::kind getKind () const 
  {
    return Field::VECTOR;
  }
  FieldVector * clone () const;
  FieldVector () : scalar (this), vector (this) { }
  void setup (Loader *, const OptionsField &, float = 0) override;
  void renderArrow (const View &, const OptionsLight &) const;
  void renderNorms (const View &, const OptionsLight &) const;
  void render (const View &, const OptionsLight &) const override;
  void reSample (const View &);
  void toggleShowVector () { opts.vector.arrow.on = ! opts.vector.arrow.on; }
  void toggleShowNorm () { opts.vector.norm.on = ! opts.vector.norm.on; }
  void reSize (const View &) override;
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
  
  enum
  {
    SCALAR=0,
    VECTOR=1
  };
  template <int N>
  class field_t
  {
  public:
    field_t (FieldVector * f) : field (f), VAID (this) {}
    field_t & operator= (const field_t & s)
    {   
      if (this != &s) 
        VAID = s.VAID;
      return *this;
    }   
    void clear ()
    {   
      VAID.clear (); 
    }   
    void setupVertexAttributes () const;
    void render (const glGrib::View &, const glGrib::OptionsLight &) const;
    FieldVector * field;
    OpenGLVertexArray<field_t> VAID;
  };  
  using scalar_t = field_t<SCALAR>;
  using vector_t = field_t<VECTOR>;
  friend class field_t<SCALAR>;
  friend class field_t<VECTOR>;
  struct
  {
    OpenGLBufferPtr<T> buffer_n, buffer_d;
    float vscale;
  } d;
  scalar_t scalar;
  vector_t vector;
protected:
  void clear () override;
};


}
