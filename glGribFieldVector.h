#pragma once

#include "glGribField.h"
#include "glGribView.h"

namespace glGrib
{

class FieldVector : public Field
{
public:
  Field::kind getKind () const 
  {
    return Field::VECTOR;
  }
  FieldVector * clone () const;
  FieldVector & operator= (const FieldVector &);
  FieldVector () { }
  FieldVector (const FieldVector &);
  void setup (Loader *, const OptionsField &, float = 0) override;
  void renderArrow (const View &, const OptionsLight &) const;
  void renderNorms (const View &, const OptionsLight &) const;
  void render (const View &, const OptionsLight &) const override;
  virtual ~FieldVector ();
  void setupVertexAttributes ();
  void reSample (const View &);
  void toggleShowVector () { opts.vector.arrow.on = ! opts.vector.arrow.on; }
  void toggleShowNorm () { opts.vector.norm.on = ! opts.vector.norm.on; }
  void resize (const View &) override;
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
      OpenGLBufferPtr buffer_n, buffer_d;
      float vscale;
    } d;
protected:
  void clear () override;
};


}
