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
  void setupVertexAttributes () const;
  void clear ();
  ~Colorbar ();
  Colorbar & operator= (const Colorbar &);
  void render (const glm::mat4 &) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
  const OptionsColorbar & getOptions () const { return opts; }
  void update (const Palette &);
private:
  void createLabels (std::vector<float> &, std::vector<float> &, 
                     std::vector<std::string> &, const std::vector<float> &, const bool);
  void updateLinear (const float, const float, std::vector<float> &, 
                     std::vector <float> &, std::vector<std::string> &);
  void updateNonLinear (const float, const float, std::vector<float> &, 
                        std::vector <float> &, std::vector<std::string> &);
  OptionsColorbar opts;
  mutable GLuint VertexArrayID;
  OpenGLBufferPtr elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  std::vector<int> rank2rgba;
  String label;
  Palette palette;
};


}
