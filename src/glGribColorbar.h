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

class Colorbar : public Object2D
{
public:
  Colorbar () : VAID (this) {}
  void setup (const OptionsColorbar &);
  void setupVertexAttributes () const;
  Colorbar & operator= (const Colorbar &);
  void render (const glm::mat4 &) const;
  const OptionsColorbar & getOptions () const { return d.opts; }
  void update (const Palette &);
  void reSize (const View &) override {}
  Object2D::side_t getSide () const override { return Object2D::LEFT; }
private:
  void createLabels (std::vector<float> &, std::vector<float> &, 
                     std::vector<std::string> &, const std::vector<float> &, const bool);
  void updateLinear (const float, const float, std::vector<float> &, 
                     std::vector <float> &, std::vector<std::string> &);
  void updateNonLinear (const float, const float, std::vector<float> &, 
                        std::vector <float> &, std::vector<std::string> &);
  struct
  {
    OptionsColorbar opts;
    OpenGLBufferPtr<unsigned int> elementbuffer;
    int nt;
    std::vector<int> rank2rgba;
    String2D<false,true> label;
    Palette palette;
  } d;
  OpenGLVertexArray<Colorbar> VAID;
};


}
