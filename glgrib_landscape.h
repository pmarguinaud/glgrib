#ifndef _GLGRIB_LANDSCAPE_H
#define _GLGRIB_LANDSCAPE_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_opengl.h"
#include "glgrib_geometry.h"
#include "glgrib_loader.h"

#include <string>

class glgrib_landscape : public glgrib_world
{
public:
  glgrib_landscape & operator=(const glgrib_landscape &);
  virtual void setup (glgrib_loader *, const glgrib_options_landscape &);
  void render (const glgrib_view &, const glgrib_options_light &) const override;
  virtual ~glgrib_landscape ();
  void set_flat (bool f) { opts.flat.on = f; }
  void toggle_flat () { opts.flat.on = ! opts.flat.on; }
  void setupVertexAttributes ();
  void resize (const glgrib_view &) override {}
  void toggle_wireframe () { opts.wireframe.on = ! opts.wireframe.on; }
  const glgrib_options_landscape & getOptions () const { return opts; }
  void setPositionOptions (const glgrib_options_landscape_position & o) { opts.lonlat.position = o; }
  float getScale () const override { return opts.scale; }
private:
  glgrib_options_landscape opts;
  glgrib_opengl_texture_ptr texture;
  glgrib_opengl_buffer_ptr heightbuffer;
};

#endif
