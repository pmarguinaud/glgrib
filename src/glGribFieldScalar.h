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
  FieldScalar () { }
  FieldScalar (const FieldScalar &);
  void setup (Loader *, const OptionsField &, float = 0) override;
  void render (const View &, const OptionsLight &) const override;
  virtual ~FieldScalar ();
  void setupVertexAttributes ();
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:
  void setupMpiView (Loader *, const OptionsField &, float = 0);
  template <typename T>
  void setupVertexAttributes ();
  template <typename T>
  void setup (Loader *, const OptionsField &, float = 0);
  GLuint VertexArrayIDpoints = 0;
  void clear () override;
};


}
