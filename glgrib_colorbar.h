#ifndef _GLGRIB_COLORBAR_H
#define _GLGRIB_COLORBAR_H

#include "glgrib_palette.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_opengl.h"
#include "glgrib_font.h"
#include "glgrib_string.h"
#include "glgrib_options.h"
#include "glgrib_program.h"

class glgrib_colorbar
{
public:
  void setup (const glgrib_options_colorbar &);
  void clear ();
  ~glgrib_colorbar ();
  glgrib_colorbar & operator= (const glgrib_colorbar &);
  void render (const glm::mat4 &, const glgrib_palette &, float, float) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
private:
  glgrib_options_colorbar opts;
  static glgrib_program program;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable glgrib_string label;
  mutable glgrib_palette pref;
};

#endif
