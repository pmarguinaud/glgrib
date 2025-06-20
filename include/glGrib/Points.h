#pragma once

#include "glGrib/View.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Options.h"
#include "glGrib/Palette.h"
#include "glGrib/View.h"
#include "glGrib/Object.h"
#include "glGrib/Palette.h"
#include "glGrib/KdTree.h"
#include "glGrib/Buffer.h"

#include <string>
#include <vector>
#include <memory>


namespace glGrib
{

class Points : public Object3D
{
public:

  Points () : VAID (this) { }
  void setupVertexAttributes () const;

  void setup (const OptionsPoints &, const std::vector<float> &, const std::vector<float> &, const std::vector<float> &);
  void render (const View &, const OptionsLight &) const override;
  void render (const View &, const OptionsLight &, const int, const int) const;
  const OptionsPoints & getOptions () const { return d.opts; }
  void reSize (const View &) override {}
  
  const glGrib::Palette & getPalette () const
  {
    return d.p;
  }

  int getNearestPoint (float, float) const;
  virtual const std::vector<float> getValue (int) const;

private:
  const bool & getVisibleRef () const override
  {
    return d.opts.visible.on;
  }
  struct
  {
    float min, max;
    OptionsPoints opts;
    int len;
    OpenGLBufferPtr<float> llsbuffer;
    glGrib::Palette p;
    std::shared_ptr<KdTree<3>> tree;
    BufferPtr<float> values;
  } d;
  OpenGLVertexArray<Points> VAID;

};


}
