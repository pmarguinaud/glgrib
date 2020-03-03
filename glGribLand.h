#pragma once

#include "glGribObject.h"
#include "glGribOpenGL.h"
#include "glGribView.h"
#include "glGribOptions.h"

#include <vector>

namespace glGrib
{

class Land : public Object
{
public:
  Land & operator=(const Land &);
  void render (const View &, const OptionsLight &) const override;
  virtual ~Land () { clear (); }
  virtual void clear ();
  virtual void setup (const OptionsLand &);
  void resize (const View &) override {}
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
  OptionsLand opts;
  typedef struct
  {
    OpenGLBufferPtr vertexbuffer, elementbuffer;
    unsigned int numberOfTriangles;
  } layer_t;
  std::vector<layer_t> d;
  std::vector<GLuint> VertexArrayID;
};


}
