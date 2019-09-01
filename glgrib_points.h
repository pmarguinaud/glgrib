#ifndef _GLGRIB_POINTS_H
#define _GLGRIB_POINTS_H

#include "glgrib_view.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_opengl.h"
#include "glgrib_options.h"
#include "glgrib_view.h"
#include <string>
#include <vector>


class glgrib_points 
{
public:
  ~glgrib_points ();
  
  glgrib_points & operator= (const glgrib_points &);
  void setupVertexAttributes ();
  void cleanup ();

  void init (const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const glgrib_view &) const;
private:
  bool ready = false;
  int len;
  GLuint VertexArrayID = 0;
  glgrib_opengl_buffer_ptr llsbuffer;
};

#endif
