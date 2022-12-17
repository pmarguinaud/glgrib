#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGrib/OpenGL.h"
#include "glGrib/Font.h"
#include "glGrib/String.h"
#include "glGrib/Options.h"
#include "glGrib/Program.h"
#include "glGrib/View.h"
#include "glGrib/Object.h"

namespace glGrib
{

class Mapscale : public Object2D
{
public:
  Mapscale () : VAID (this) {}
  void setup (const OptionsMapscale &);
  void setupVertexAttributes () const;
  void render (const glm::mat4 &) const override;
  const OptionsMapscale & getOptions () const { return opts; }
  void reSize (const View &) override;
  Object2D::side_t getSide () const override { return Object2D::LEFT; }
private:
  OptionsMapscale opts;
  OpenGLVertexArray<Mapscale> VAID;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  int nt;
  String2D<0,1> label;
  std::string label_str;
  double frac1;
};


}
