#pragma once

#include "glGribField.h"

namespace glGrib
{

template <int N>
class FieldScalar : public FieldPacked<N>
{
public:
  using T = typename FieldPackingType<N>::type;
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
    return (int)this->opts.path.size ();
  }
private:
  void setupMpiView (Loader *, const OptionsField &, float = 0);
  void clear () override;

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
    virtual void setupVertexAttributes () const = 0;
    virtual void render (const glGrib::View & view) const = 0;
    virtual std::string getProgramName () const = 0;
    Program * getProgram () const
    {
      return glGrib::Program::load (getProgramName ());
    }
    FieldScalar * field;
    OpenGLVertexArray<field_t> VAID;
  };

  class scalar_t : public field_t
  {
  public:
    scalar_t (FieldScalar * f) : field_t (f) {}
    void setupVertexAttributes () const override;
    void render (const glGrib::View & view) const override;
    std::string getProgramName () const override
    {
      return "SCALAR";
    }
  };

  class points_t : public field_t
  {
  public:
    points_t (FieldScalar * f) : field_t (f) {}
    void setupVertexAttributes () const override;
    void render (const glGrib::View & view) const override;
    std::string getProgramName () const override
    {
      return "SCALAR_POINTS";
    }
  };

  scalar_t scalar;
  points_t points;

  OpenGLBufferPtr<float> mpivbuffer;
  OpenGLBufferPtr<T> colorbuffer;
};


}
