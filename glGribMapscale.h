#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpengl.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"
#include "glGribView.h"

class glGribMapscale
{
public:
  void setup (const glGribOptionsMapscale &);
  void clear ();
  ~glGribMapscale ();
  glGribMapscale & operator= (const glGribMapscale &);
  void render (const glm::mat4 &, const glGribView &) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const glGribOptionsMapscale & getOptions () const { return opts; }
private:
  glGribOptionsMapscale opts;
  static glGribProgram program;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable glGribString label;
  mutable std::string label_str;
};

