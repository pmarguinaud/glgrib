#pragma once

#include "glGrib/World.h"
#include "glGrib/Options.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Geometry.h"
#include "glGrib/Loader.h"

#include <string>

namespace glGrib
{

class Landscape : public World
{
public:
  Landscape () : VAID (this) {}
  virtual void setup (Loader *, const OptionsLandscape &);
  void render (const View &, const OptionsLight &) const override;
  void setupVertexAttributes () const;
  void reSize (const View &) override {}
  void setWireFrameOption (bool wireframe) 
  { 
    d.opts.wireframe.on = wireframe; 
  }
  const OptionsLandscape & getOptions () const { return d.opts; }
  void setPositionOptions (const OptionsLandscapePosition & o) 
  { 
    d.opts.lonlat.position = o; 
  }
  void setFlatOption (bool flat) 
  {
    d.opts.flat.on = flat; 
  }
  float getScale () const override { return d.opts.scale; }
private:
  const bool & getVisibleRef () const override
  {
    return d.opts.visible.on;
  }
  struct
  {
    OptionsLandscape opts;
    OpenGLTexturePtr texture;
    OpenGLBufferPtr<float> heightbuffer;
  } d;
  OpenGLVertexArray<Landscape> VAID;
};


}
