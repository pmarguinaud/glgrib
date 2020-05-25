#pragma once

#include "glGribWorld.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribGeometry.h"
#include "glGribLoader.h"

#include <string>

namespace glGrib
{

class Landscape : public World
{
public:
  Landscape () : VAID (this) {}
  Landscape & operator=(const Landscape &);
  virtual void setup (Loader *, const OptionsLandscape &);
  void render (const View &, const OptionsLight &) const override;
  virtual ~Landscape ();
  void setFlat (bool f) { d.opts.flat.on = f; }
  void toggleFlat () { d.opts.flat.on = ! d.opts.flat.on; }
  void setupVertexAttributes () const;
  void reSize (const View &) override {}
  void toggleWireframe () { d.opts.wireframe.on = ! d.opts.wireframe.on; }
  const OptionsLandscape & getOptions () const { return d.opts; }
  void setPositionOptions (const OptionsLandscapePosition & o) { d.opts.lonlat.position = o; }
  float getScale () const override { return d.opts.scale; }
private:
  struct
  {
    OptionsLandscape opts;
    OpenGLTexturePtr texture;
    OpenGLBufferPtr heightbuffer;
  } d;
  mutable OpenGLVertexArray<Landscape> VAID;
};


}
