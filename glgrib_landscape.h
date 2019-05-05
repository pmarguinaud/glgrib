#ifndef _GLGRIB_LANDSCAPE_H
#define _GLGRIB_LANDSCAPE_H

#include "glgrib_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_landscape : public glgrib_world
{
public:
  virtual void init (const glgrib_options &, const glgrib_geometry *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  GLuint textureID;
  void render (const glgrib_view *) const;
  virtual ~glgrib_landscape ();
  virtual bool use_alpha () { return false; }
  bool flat = false;
  void set_flat (bool f) { flat = f; }
  void toggle_flat () { flat = ! flat; program = NULL; }
  void getLightPos (float * x, float * y)
    {
      *x = lightx;
      *y = lighty;
    }
  void setLightPos (float x, float y)
    { 
      lightx = x;
      lighty = y;
    }
  void setLight ()
    {
      light = true;
    }
  void unsetLight () { light = false; }
  bool hasLight () { return light; }

private:
  float lightx = 0., lighty = 0.;
  bool light = false;
};

#endif
