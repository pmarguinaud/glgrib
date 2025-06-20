#pragma once

#include "glGrib/Object.h"
#include "glGrib/Options.h"
#include "glGrib/String.h"
#include "glGrib/OpenGL.h"

namespace glGrib
{

class Ticks : public Object2D
{
public:
  Ticks () : ticks (this), frame (this) {}
  Ticks & operator=(const Ticks &);
  void setup (const OptionsTicks &);
  void render (const glm::mat4 &) const;
  void reSize (const View &) override;
  const OptionsTicks & getOptions () const { return opts; }
  Object2D::side_t getSide () const override { return Object2D::LEFT; }
private:
  void createStr 
    (const OptionsTicksSide &,
     StringTypes::align_t, const View &, 
     std::vector<std::string> &, std::vector<float> &, 
     std::vector<float> &, std::vector<float> &,
     std::vector<StringTypes::align_t> &);

  // No need to copy these

  String2D<0,0> labels;
  OptionsTicks opts;
  int width = 0, height = 0;
  OptionsView vopts;

  enum
  {
    TICKS=0,
    FRAME=1
  };

  template <int N>
  class item_t
  {
  public:
    explicit item_t (Ticks * t) : ticks (t), VAID (this) {}
    item_t & operator= (const item_t & it)
    {
      if (this != &it)
        VAID = it.VAID;
      return *this;
    }
    void render (const glm::mat4 &) const;
    void setupVertexAttributes () const;
    Ticks * ticks;
    OpenGLVertexArray<item_t> VAID;
  };

  using ticks_t = item_t<TICKS>;
  using frame_t = item_t<FRAME>;
  friend class item_t<TICKS>;
  friend class item_t<FRAME>;
  OpenGLBufferPtr<glm::vec3> vertexbuffer;
  unsigned int numberOfTicks;
  ticks_t ticks;
  frame_t frame;
};


}
