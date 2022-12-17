#pragma once

#include "glGrib/Field.h"

namespace glGrib
{

template <int N>
class FieldScalar : public FieldPacked<N>
{
public:
  explicit FieldScalar (const Field::Privatizer) : scalar (this), points (this) { }
  void setup (const Field::Privatizer, Loader *, const OptionsField &, float = 0) override;
  FieldScalar (const FieldScalar &) = delete;

  using T = typename FieldPackingType<N>::type;

  Field::kind getKind () const 
  {
    return Field::SCALAR;
  }
  FieldScalar * clone () const;
  void render (const View &, const OptionsLight &) const override;
  bool useColorBar () const override { return true; }
  int getSlotMax () const override
  {
    return (int)this->opts.path.size ();
  }
private:

  void setupMpiView (Loader *, const OptionsField &, float = 0);

  class field_t
  {
  public:
    explicit field_t (FieldScalar * f) : field (f), VAID (this) {}
    field_t & operator= (const field_t & s)
    {
      if (this != &s)
        VAID = s.VAID;
      return *this;
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
    explicit scalar_t (FieldScalar * f) : field_t (f) {}
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
    explicit points_t (FieldScalar * f) : field_t (f) {}
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
