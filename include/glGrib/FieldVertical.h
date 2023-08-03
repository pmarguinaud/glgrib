#pragma once

#include "glGrib/Field.h"

namespace glGrib
{

class FieldVertical : public Field
{
public:
  explicit FieldVertical (const Field::Privatizer) { }
  void setup (const Field::Privatizer, Loader *, const OptionsField &) override;
  Field::kind getKind () const 
  {
    return Field::VERTICAL;
  }
  FieldVertical * clone () const;
  void render (const View &, const OptionsLight &) const override;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return 1;
  }
  void setupVertexAttributes () const;
  void reSize (const View &) override {}
  float getScale () const override { return 1; }
  FieldVertical (const FieldVertical &) = delete;
private:
  bool _visible = true;
  const bool & getVisibleRef () const override
  {
    return _visible;
  }

  int numberOfPoints;
  unsigned int numberOfTriangles;
  int Nx = -1, Nz = -1;

  class vertical_vaid
  {
  public:
    vertical_vaid () : VAID (this) {}
    OpenGLBufferPtr<float> lonlatbuffer;
    OpenGLBufferPtr<float> valuesbuffer;
    OpenGLBufferPtr<float> heightbuffer;
    OpenGLVertexArray<vertical_vaid> VAID;
    void setupVertexAttributes () const;
  };

  vertical_vaid d;
};


}
