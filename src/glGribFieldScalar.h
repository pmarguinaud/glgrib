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
  FieldScalar () : scalar (this), points (this) { }
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
    void setupVertexAttributes () const;
    void setupVertexAttributes () const;
    void render (const glGrib::View & view) const;
    std::string getProgramName () const;
    Program * getProgram () const
    {
      return glGrib::Program::load (getProgramName ());
    }
    FieldScalar * field;
    mutable OpenGLVertexArray<field_t> VAID;
  };
  class scalar_t
  {
  public:
    scalar_t (FieldScalar * f) : field (f), VAID (this) {}
    scalar_t & operator= (const scalar_t & s)
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
    void setupVertexAttributes () const;
    void setupVertexAttributes () const;
    void render (const glGrib::View & view) const;
    Program * getProgram () const
    {
      return glGrib::Program::load ("SCALAR");
    }
    FieldScalar * field;
    mutable OpenGLVertexArray<scalar_t> VAID;
  };
  class points_t
  {
  public:
    points_t (FieldScalar * f) : field (f), VAID (this) {}
    points_t & operator= (const points_t & p)
    {
      if (this != &p)
        VAID = p.VAID;
      return *this;
    }
    void clear ()
    {
      VAID.clear ();
    }
    template <typename T>
    void setupVertexAttributes () const;
    void setupVertexAttributes () const;
    void render (const glGrib::View & view) const;
    Program * getProgram () const
    {
      return glGrib::Program::load ("SCALAR_POINTS");
    }
    FieldScalar * field;
    mutable OpenGLVertexArray<points_t> VAID;
  };
  friend class scalar_t;
  friend class points_t;
  scalar_t scalar;
  points_t points;
};


}
