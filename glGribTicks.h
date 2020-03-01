#pragma once

#include "glGribObject.h"
#include "glGribOptions.h"
#include "glGribString.h"
#include "glGribOpengl.h"

class glGribTicks 
{
public:
  glGribTicks & operator=(const glGribTicks &);
  void setup (const glgrib_options_ticks &);
  void render (const glm::mat4 &) const;
  void resize (const glGribView &);
  const glgrib_options_ticks & getOptions () const { return opts; }
  void setupVertexAttributes ();
  void clear ();
  ~glGribTicks ();
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
    (const glgrib_options_ticks_side &,
     glGribString::align_t, const glGribView &, 
     std::vector<std::string> &, std::vector<float> &, 
     std::vector<float> &, std::vector<float> &,
     std::vector<glGribString::align_t> &);
  glGribString labels;
  glgrib_options_ticks opts;
  int width = 0, height = 0;
  glgrib_options_view vopts;
  bool ready = false;
  GLuint VertexArrayID = 0;
  GLuint VertexArrayID_frame = 0;
  glgrib_opengl_buffer_ptr vertexbuffer;
  unsigned int numberOfTicks;
};

