#ifndef _GLGRIB_POINTS_H
#define _GLGRIB_POINTS_H

#include "glgrib_view.h"
#include "glgrib_opengl.h"
#include "glgrib_options.h"
#include "glgrib_palette.h"
#include "glgrib_view.h"
#include "glgrib_object.h"
#include <string>
#include <vector>


class glgrib_points : public glgrib_object
{
public:
  ~glgrib_points ();
  
  glgrib_points & operator= (const glgrib_points &);
  void setupVertexAttributes ();
  virtual void clear ();

  void setup (const glgrib_options_points &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const override;
  const glgrib_options_points & getOptions () const { return d.opts; }
  virtual void resize (const glgrib_view &) override {}
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

#endif
