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
  Ticks () : VAID_ticks (this), VAID_frame (this) {}
  Ticks & operator=(const Ticks &);
  void setup (const OptionsTicks &);
  void render (const glm::mat4 &) const;
  void reSize (const View &);
  const OptionsTicks & getOptions () const { return opts; }
  void setupVertexAttributes () const;
  void clear ();
  ~Ticks ();
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
  String labels;
  OptionsTicks opts;
  int width = 0, height = 0;
  OptionsView vopts;
  bool ready = false;
  mutable OpenGLVertexArray<Ticks> VAID_ticks;
  mutable OpenGLVertexArray<Ticks> VAID_frame;
  OpenGLBufferPtr vertexbuffer;
  unsigned int numberOfTicks;
};


}
