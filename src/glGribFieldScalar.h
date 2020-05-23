#pragma once

#include "glGribField.h"

namespace glGrib
{

class FieldScalar : public Field
{
public:
  Field::kind getKind () const 
  {
    return Field::SCALAR;
  }
  FieldScalar * clone () const;
  FieldScalar & operator= (const FieldScalar &);
  FieldScalar () : VAID_scalar (this), VAID_points (this) { }
  FieldScalar (const FieldScalar &);
  void setup (Loader *, const OptionsField &, float = 0) override;
  void render (const View &, const OptionsLight &) const override;
  virtual ~FieldScalar ();
  void setupVertexAttributes () const;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:
  void setupMpiView (Loader *, const OptionsField &, float = 0);
  template <typename T>
  void setupVertexAttributes () const;
  template <typename T>
  void setup (Loader *, const OptionsField &, float = 0);
  void clear () override;
  mutable OpenGLVertexArray<FieldScalar> VAID_scalar;
  mutable OpenGLVertexArray<FieldScalar> VAID_points;
};


}
