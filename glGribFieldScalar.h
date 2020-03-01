#pragma once

#include "glGribField.h"

class glGribFieldScalar : public glGribField
{
public:
  glGribField::kind getKind () const 
  {
    return glGribField::SCALAR;
  }
  glGribFieldScalar * clone () const;
  glGribFieldScalar & operator= (const glGribFieldScalar &);
  glGribFieldScalar () { }
  glGribFieldScalar (const glGribFieldScalar &);
  void setup (glGribLoader *, const glGribOptionsField &, float = 0) override;
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribFieldScalar ();
  void setupVertexAttributes ();
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:
  void setupMpiView (glGribLoader *, const glGribOptionsField &, float = 0);
  template <typename T>
  void setupVertexAttributes ();
  template <typename T>
  void setup (glGribLoader *, const glGribOptionsField &, float = 0);
  GLuint VertexArrayIDpoints = 0;
  void clear () override;
};

