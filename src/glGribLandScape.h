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
  void setFlat (bool f) { opts.flat.on = f; }
  void toggleFlat () { opts.flat.on = ! opts.flat.on; }
  void setupVertexAttributes () const;
  void reSize (const View &) override {}
  void toggleWireframe () { opts.wireframe.on = ! opts.wireframe.on; }
  const OptionsLandscape & getOptions () const { return opts; }
  void setPositionOptions (const OptionsLandscapePosition & o) { opts.lonlat.position = o; }
  float getScale () const override { return opts.scale; }
private:
  OptionsLandscape opts;
  OpenGLTexturePtr texture;
  OpenGLBufferPtr heightbuffer;
  mutable OpenGLVertexArray<Landscape> VAID;
};


}
