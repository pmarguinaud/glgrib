#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"
#include "glGribView.h"

namespace glGrib
{

class Mapscale
{
public:
  Mapscale () : VAID (this) {}
  void setup (const OptionsMapscale &);
  void setupVertexAttributes () const;
  void render (const glm::mat4 &, const View &) const;
  const OptionsMapscale & getOptions () const { return opts; }
private:
  OptionsMapscale opts;
  OpenGLVertexArray<Mapscale> VAID;
  OpenGLBufferPtr<unsigned int> elementbuffer;
  bool ready = false;
  int nt;
  mutable String2D label;
  mutable std::string label_str;
};


}
