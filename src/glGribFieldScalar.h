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
  FieldScalar () : scalar (this), points (this) { }
  void setup (Loader *, const OptionsField &, float = 0) override;
  void render (const View &, const OptionsLight &) const override;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)opts.path.size ();
  }
private:
  void setupMpiView (Loader *, const OptionsField &, float = 0);
  template <typename T>
  void setup (Loader *, const OptionsField &, float = 0);
  void clear () override;
  enum
  {
    SCALAR=0,
    POINTS=1
  };
  template <int N>
  class field_t
  {
  public:
    field_t (FieldScalar * f) : field (f), VAID (this) {}
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
    template <typename T>
    void setupVertexAttributes_typed () const;
    void setupVertexAttributes () const;
    void render (const glGrib::View & view) const;
    std::string getProgramName () const;
    Program * getProgram () const
    {
      return glGrib::Program::load (getProgramName ());
    }
    FieldScalar * field;
    OpenGLVertexArray<field_t> VAID;
  };
  using scalar_t = field_t<SCALAR>;
  using points_t = field_t<POINTS>;
  friend class field_t<SCALAR>;
  friend class field_t<POINTS>;
  scalar_t scalar;
  points_t points;
};


}
