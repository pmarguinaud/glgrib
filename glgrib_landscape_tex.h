#ifndef _GLGRIB_LANDSCAPE_TEX_H
#define _GLGRIB_LANDSCAPE_TEX_H

#include "glgrib_landscape.h"
#include "glgrib_coords_world.h"

class glgrib_landscape_tex : public glgrib_landscape
{
public:
  virtual void init (const char *, const glgrib_coords_world *);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  GLuint textureID;
  void render (const glgrib_view *) const;
  ~glgrib_landscape_tex ();
};

#endif
