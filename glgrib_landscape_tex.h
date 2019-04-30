#ifndef _GLGRIB_LANDSCAPE_TEX_H
#define _GLGRIB_LANDSCAPE_TEX_H

#include "glgrib_landscape.h"
#include "glgrib_coords_world.h"
#include "glgrib_options.h"
#include "glgrib_geometry.h"

#include <string>

class glgrib_landscape_tex : public glgrib_landscape
{
public:
  virtual void init (const glgrib_options &, const glgrib_coords_world *, const glgrib_geometry &);
  virtual glgrib_program_kind get_program_kind () const 
    { 
      return GLGRIB_PROGRAM_FLAT_TEX;
    }
  GLuint textureID;
  void render (const glgrib_view *) const;
  ~glgrib_landscape_tex ();
};

#endif
