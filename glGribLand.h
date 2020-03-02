#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribOptions.h"

#include <vector>

class glGribLand : public glGribObject
{
public:
  glGribLand & operator=(const glGribLand &);
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribLand () { clear (); }
  virtual void clear ();
  virtual void setup (const glGribOptionsLand &);
  void resize (const glGribView &) override {}
  void setupVertexAttributes ();
  float getScale () const override { return opts.layers[0].scale; }
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
  glGribOptionsLand opts;
  typedef struct
  {
    glgrib_opengl_buffer_ptr vertexbuffer, elementbuffer;
    unsigned int numberOfTriangles;
  } layer_t;
  std::vector<layer_t> d;
  std::vector<GLuint> VertexArrayID;
};

