#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"
#include "glGribOpenGL.h"

namespace glGrib
{

class Ticks 
{
public:
  Ticks () : ticks (this), frame (this) {}
  Ticks & operator=(const Ticks &);
  void setup (const OptionsTicks &);
  void render (const glm::mat4 &) const;
  void reSize (const View &);
  const OptionsTicks & getOptions () const { return opts; }
  void clear ();
  void setReady ()
  {
    ready = true;
  }
  bool isReady () const
  {
    return ready;
  }
private:
  void createStr 
    (const OptionsTicksSide &,
     String::align_t, const View &, 
     std::vector<std::string> &, std::vector<float> &, 
     std::vector<float> &, std::vector<float> &,
     std::vector<String::align_t> &);

  // No need to copy these

  String labels;
  OptionsTicks opts;
  int width = 0, height = 0;
  OptionsView vopts;
  bool ready = false;

  enum
  {
    TICKS=0,
    FRAME=1
  };

  template <int N>
  class item_t
  {
  public:
    item_t (Ticks * t) : ticks (t), VAID (this) {}
    item_t & operator= (const item_t & it)
    {
      if (this != &it)
        VAID = it.VAID;
      return *this;
    }
    void clear ()
    {
      VAID.clear ();
    }
    void render (const glm::mat4 &) const;
    void setupVertexAttributes () const;
    Ticks * ticks;
    mutable OpenGLVertexArray<item_t> VAID;
  };

  using ticks_t = item_t<TICKS>;
  using frame_t = item_t<FRAME>;
  friend class item_t<TICKS>;
  friend class item_t<FRAME>;
  OpenGLBufferPtr vertexbuffer;
  unsigned int numberOfTicks;
  ticks_t ticks;
  frame_t frame;
};


}
