#pragma once

#include "glGribField.h"
#include "glGribView.h"

class glGribFieldVector : public glGribField
{
public:
  glGribField::kind getKind () const 
  {
    return glGribField::VECTOR;
  }
  glGribFieldVector * clone () const;
  glGribFieldVector & operator= (const glGribFieldVector &);
  glGribFieldVector () { }
  glGribFieldVector (const glGribFieldVector &);
  void setup (glGribLoader *, const glGribOptionsField &, float = 0) override;
  void renderArrow (const glGribView &, const glGribOptionsLight &) const;
  void renderNorms (const glGribView &, const glGribOptionsLight &) const;
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribFieldVector ();
  void setupVertexAttributes ();
  void reSample (const glGribView &);
  void toggleShowVector () { opts.vector.arrow.on = ! opts.vector.arrow.on; }
  void toggleShowNorm () { opts.vector.norm.on = ! opts.vector.norm.on; }
  void resize (const glGribView &) override;
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
      glGribOpenGLBufferPtr buffer_n, buffer_d;
      float vscale;
    } d;
protected:
  void clear () override;
};

