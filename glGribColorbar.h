#pragma once

#include "glGribPalette.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpengl.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"

class glGribColorbar
{
public:
  void setup (const glgrib_options_colorbar &);
  void clear ();
  ~glGribColorbar ();
  glGribColorbar & operator= (const glGribColorbar &);
  void render (const glm::mat4 &, const glGribPalette &, float, float) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const glgrib_options_colorbar & getOptions () const { return opts; }
private:
  glgrib_options_colorbar opts;
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

