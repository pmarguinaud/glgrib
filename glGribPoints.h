#pragma once

#include "glGribView.h"
#include "glGribOpengl.h"
#include "glGribOptions.h"
#include "glGribPalette.h"
#include "glGribView.h"
#include "glGribObject.h"
#include <string>
#include <vector>


class glGribPoints : public glGribObject
{
public:
  ~glGribPoints ();
  
  glGribPoints & operator= (const glGribPoints &);
  void setupVertexAttributes ();
  virtual void clear ();

  void setup (const glgrib_options_points &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const glGribView &, const glgrib_options_light &) const override;
  const glgrib_options_points & getOptions () const { return d.opts; }
  void resize (const glGribView &) override {}
private:
  struct
  {
    float min, max;
    glgrib_options_points opts;
    int len;
    glgrib_opengl_buffer_ptr llsbuffer;
  } d;
  GLuint VertexArrayID = 0;
};

