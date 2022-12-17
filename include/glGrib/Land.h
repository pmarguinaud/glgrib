#pragma once

#include "glGrib/Object.h"
#include "glGrib/OpenGL.h"
#include "glGrib/View.h"
#include "glGrib/Options.h"

#include <vector>

namespace glGrib
{

class Land : public Object3D
{
public:
  using int_v = std::vector<int>;
  void render (const View &, const OptionsLight &) const override;
  virtual void setup (const OptionsLand &);
  void reSize (const View &) override {}
  float getScale () const override { return opts.layers[0].scale; }
  const OptionsLand & getOptions () const
  {
    return opts;
  }
private:
  const bool & getVisibleRef () const override
  {
    return opts.visible.on;
  }
  OptionsLand opts;
  class layer_t
  {
  public:
    layer_t () : VAID (this) {}
    layer_t (const layer_t & layer) : VAID (this)
    {
      d = layer.d;
    }
    void triangulate (int_v *, int_v *,
                      int_v *, int_v *,
                      const std::vector<unsigned int> &,
                      std::vector<float> *, int_v *,
                      std::vector<unsigned int> *);
    void subdivide (const int_v &, const int_v &,
                    const int_v &, const int_v &,
                    std::vector<unsigned int> *, std::vector<float> *,
                    const float);
    void setupVertexAttributes () const;
    void render (const glGrib::OptionsLandLayer &) const;
    void setup (const glGrib::OptionsLandLayer &);
    struct
    {
      OpenGLBufferPtr<float> vertexbuffer;
      OpenGLBufferPtr<unsigned int> elementbuffer;
      unsigned int numberOfTriangles;
    } d;
    OpenGLVertexArray<layer_t> VAID;
  };
  std::vector<layer_t> layers;
};


}
