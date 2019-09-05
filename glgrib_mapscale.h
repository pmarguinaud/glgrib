#ifndef _GLGRIB_MAPSCALE_H
#define _GLGRIB_MAPSCALE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_opengl.h"
#include "glgrib_font.h"
#include "glgrib_string.h"
#include "glgrib_options.h"
#include "glgrib_program.h"
#include "glgrib_view.h"

class glgrib_mapscale
{
public:
  void setup (const glgrib_options_mapscale &);
  void clear ();
  ~glgrib_mapscale ();
  glgrib_mapscale & operator= (const glgrib_mapscale &);
  void render (const glm::mat4 &, const glgrib_view &) const;
  void toggleHidden () { hidden = ! hidden; }
  bool getHidden () const { return hidden; }
private:
  glgrib_options_mapscale opts;
  static glgrib_program program;
  GLuint VertexArrayID;
  GLuint elementbuffer;
  bool ready = false;
  bool hidden = false;
  int nt;
  mutable glgrib_string label;
  mutable std::string label_str;
};

#endif
