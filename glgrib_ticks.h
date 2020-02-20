#pragma once

#include "glgrib_object.h"
#include "glgrib_options.h"
#include "glgrib_string.h"

class glgrib_ticks 
{
public:
  glgrib_ticks & operator=(const glgrib_ticks &);
  void setup (const glgrib_options_ticks &);
  void render (const glm::mat4 &) const;
  void resize (const glgrib_view &);
  const glgrib_options_ticks & getOptions () const { return opts; }
  void setupVertexAttributes ();
  void clear ();
  ~glgrib_ticks ();
  void setReady ()
  {
    ready = true;
  }
  bool isReady () const
  {
    return ready;
  }
private:
  void createStr (glgrib_string::align_t, const glgrib_view &, glgrib_string *);
  glgrib_string labels_n, labels_s, labels_e, labels_w;
  glgrib_options_ticks opts;
  int width = 0, height = 0;
  glgrib_options_view vopts;
  bool ready = false;
};

