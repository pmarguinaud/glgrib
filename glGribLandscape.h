#pragma once

#include "glGribWorld.h"
#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribGeometry.h"
#include "glGribLoader.h"

#include <string>

class glGribLandscape : public glGribWorld
{
public:
  glGribLandscape & operator=(const glGribLandscape &);
  virtual void setup (glGribLoader *, const glGribOptionsLandscape &);
  void render (const glGribView &, const glGribOptionsLight &) const override;
  virtual ~glGribLandscape ();
  void setFlat (bool f) { opts.flat.on = f; }
  void toggleFlat () { opts.flat.on = ! opts.flat.on; }
  void setupVertexAttributes ();
  void resize (const glGribView &) override {}
  void toggleWireframe () { opts.wireframe.on = ! opts.wireframe.on; }
  const glGribOptionsLandscape & getOptions () const { return opts; }
  void setPositionOptions (const glGribOptionsLandscapePosition & o) { opts.lonlat.position = o; }
  float getScale () const override { return opts.scale; }
private:
  glGribOptionsLandscape opts;
  glgrib_opengl_texture_ptr texture;
  glGribOpenGLBufferPtr heightbuffer;
};

