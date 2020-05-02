#pragma once

#include "glGribPalette.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glGribOpenGL.h"
#include "glGribFont.h"
#include "glGribString.h"
#include "glGribOptions.h"
#include "glGribProgram.h"

namespace glGrib
{

class Colorbar
{
public:
  void setup (const OptionsColorbar &);
  void clear ();
  ~Colorbar ();
  Colorbar & operator= (const Colorbar &);
  void render (const glm::mat4 &, const Palette &) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const OptionsColorbar & getOptions () const { return opts; }
private:
  void mute () const;
  void muteLinear (const float, const float, std::vector<float> &, 
                   std::vector <float> &, std::vector<std::string> &) const;
  void muteNonLinear (const float, const float, std::vector<float> &, 
                      std::vector <float> &, std::vector<std::string> &) const;
  OptionsColorbar opts;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable std::vector<int> rank2rgba;
  mutable String label;
  mutable Palette pref;
};


}
