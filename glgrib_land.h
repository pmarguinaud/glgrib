#ifndef _GLGRIB_LAND_H
#define _GLGRIB_LAND_H

#include "glgrib_object.h"
#include "glgrib_opengl.h"
#include "glgrib_view.h"
#include "glgrib_options.h"

#include <vector>

class glgrib_land : public glgrib_object
{
public:
  glgrib_land & operator=(const glgrib_land &);
  virtual void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_land () { clear (); }
  virtual void clear ();
  virtual void setup (const glgrib_options_land &);
  virtual void resize (const glgrib_view &) {}
  void setupVertexAttributes ();
  virtual float getScale () const { return opts.layers[0].scale; }
private:
  void triangulate (std::vector<int> *, std::vector<int> *,
                    std::vector<int> *, std::vector<int> *,
                    const std::vector<unsigned int> &,
                    std::vector<float> *, std::vector<int> *,
                    std::vector<unsigned int> *);
  void subdivide (const std::vector<int> &, const std::vector<int> &,
                  const std::vector<int> &, const std::vector<int> &,
                  std::vector<unsigned int> *, std::vector<float> *,
                  const float);
private:
  glgrib_options_land opts;
  typedef struct
  {
    glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
    unsigned int numberOfTriangles;
  } layer_t;
  std::vector<layer_t> d;
  std::vector<GLuint> VertexArrayID;
};

#endif
