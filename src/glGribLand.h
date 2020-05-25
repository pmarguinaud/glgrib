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
  void reSize (const View &) override {}
  void setupVertexAttributes () const;
  float getScale () const override { return opts.layers[0].scale; }
private:
  OptionsLand opts;
  class layer_t
  {
  public:
    layer_t () : VAID (this) {}
    layer_t (const layer_t & layer) : VAID (this)
    {
      d = layer.d;
    }
    void clear ()
    {
      VAID.clear ();
    }
    void triangulate (std::vector<int> *, std::vector<int> *,
                      std::vector<int> *, std::vector<int> *,
                      const std::vector<unsigned int> &,
                      std::vector<float> *, std::vector<int> *,
                      std::vector<unsigned int> *);
    void subdivide (const std::vector<int> &, const std::vector<int> &,
                    const std::vector<int> &, const std::vector<int> &,
                    std::vector<unsigned int> *, std::vector<float> *,
                    const float);
    void setupVertexAttributes () const;
    void render (const glGrib::OptionsLandLayer &) const;
    void setup (const glGrib::OptionsLandLayer &);
    struct
    {
      OpenGLBufferPtr vertexbuffer, elementbuffer;
      unsigned int numberOfTriangles;
    } d;
    mutable OpenGLVertexArray<layer_t> VAID;
  };
  std::vector<layer_t> layers;
};


}
