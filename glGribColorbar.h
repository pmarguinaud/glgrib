#pragma once

#include "glGribPalette.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"

class glGribColorbar
{
public:
  void setup (const glGribOptionsColorbar &);
  void clear ();
  ~glGribColorbar ();
  glGribColorbar & operator= (const glGribColorbar &);
  void render (const glm::mat4 &, const glGribPalette &, float, float) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const glGribOptionsColorbar & getOptions () const { return opts; }
private:
  glGribOptionsColorbar opts;
  static glGribProgram program;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable std::vector<int> rank2rgba;
  mutable glGribString label;
  mutable glGribPalette pref;
};

