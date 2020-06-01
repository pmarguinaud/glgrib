#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"
#include "glGribView.h"
#include "glGribObject.h"

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
private:
  OptionsMapscale opts;
  OpenGLVertexArray<Mapscale> VAID;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  int nt;
  String2D label;
  std::string label_str;
  double frac1;
};


}
