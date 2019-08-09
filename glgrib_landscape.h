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
  virtual void init (glgrib_loader *, const glgrib_options_landscape &);
  void render (const glgrib_view &, const glgrib_options_light &) const;
  virtual ~glgrib_landscape ();
  virtual bool use_alpha () { return false; }
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; }
  void setupVertexAttributes ();
  virtual void resize (const glgrib_view &) {}
private:
  bool flat = false;
  glgrib_opengl_texture_ptr texture;
};

#endif
