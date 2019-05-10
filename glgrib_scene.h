#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <vector>
#include <set>

#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_landscape.h"
#include "glgrib_grid.h"
#include "glgrib_field.h"
#include "glgrib_coastlines.h"

class glgrib_scene
{
public:
  void display () const;
  void display_obj (const glgrib_object *) const;
  std::vector<glgrib_field> fieldlist;
  glgrib_view view;
  glgrib_landscape landscape;
  glgrib_coastlines coastlines;
  glgrib_grid grid;

  glgrib_field * getCurrentField () 
    { 
      return currentFieldRank < fieldlist.size () ? &fieldlist[currentFieldRank] : NULL; 
    }
  glgrib_field_display_options * getCurrentFieldOpts () 
    { 
      return currentFieldRank < fieldlist.size () ? &fieldlist[currentFieldRank].dopts : NULL; 
    }

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
  void update ();
  bool rotate_earth = false;
  bool rotate_light = false;
  void toggleMovie () { movie = ! movie; movie_index = 0; }
  void setMovie () { movie = true; movie_index = 0; }
  void setCurrentFieldRank (int r) { currentFieldRank = r; }

private:
  float lightx = 0., lighty = 0.;
  bool light = false;
  bool movie = false;
  int movie_index = 0;
  double movie_time = 0;
  void setLightShader (GLuint) const;
  int currentFieldRank = 0;
};

#endif
