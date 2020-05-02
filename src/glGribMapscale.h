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
  void setup (const OptionsMapscale &);
  void clear ();
  ~Mapscale ();
  Mapscale & operator= (const Mapscale &);
  void render (const glm::mat4 &, const View &) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const OptionsMapscale & getOptions () const { return opts; }
private:
  OptionsMapscale opts;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable String label;
  mutable std::string label_str;
};


}
