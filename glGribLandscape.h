#pragma once

#include "glGribWorld.h"
#include "glGribOptions.h"
#include "glGribOpengl.h"
#include "glGribGeometry.h"
#include "glGribLoader.h"

#include <string>

class glGribLandscape : public glGribWorld
{
public:
  glGribLandscape & operator=(const glGribLandscape &);
  virtual void setup (glGribLoader *, const glgrib_options_landscape &);
  void render (const glGribView &, const glgrib_options_light &) const override;
  virtual ~glGribLandscape ();
  void set_flat (bool f) { opts.flat.on = f; }
  void toggle_flat () { opts.flat.on = ! opts.flat.on; }
  void setupVertexAttributes ();
  void resize (const glGribView &) override {}
  void toggle_wireframe () { opts.wireframe.on = ! opts.wireframe.on; }
  const glgrib_options_landscape & getOptions () const { return opts; }
  void setPositionOptions (const glgrib_options_landscape_position & o) { opts.lonlat.position = o; }
  float getScale () const override { return opts.scale; }
private:
  glgrib_options_landscape opts;
  glgrib_opengl_texture_ptr texture;
  glgrib_opengl_buffer_ptr heightbuffer;
};

