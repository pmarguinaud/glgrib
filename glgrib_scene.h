#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <vector>
#include <set>

#include "glgrib_font.h"
#include "glgrib_string.h"
#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_landscape.h"
#include "glgrib_grid.h"
#include "glgrib_field.h"
#include "glgrib_coastlines.h"
#include "glgrib_colorbar.h"

#include <set>

class glgrib_scene
{
public:
  glgrib_scene () {}
  glgrib_options opts;
  void init (const glgrib_options &);
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

  void getLightPos (float * lon, float * lat) const
    {
      *lon = light.lon;
      *lat = light.lat;
    }
  void setLightPos (float lon, float lat) 
    { 
      light.lon = lon;
      light.lat = lat;
    }
  void setLight ()
    {
      light.on = true;
    }
  void unsetLight () { light.on = false; }
  bool hasLight () const { return light.on; }
  void update ();
  bool rotate_earth = false;
  void toggleMovie () { movie = ! movie; movie_index = 0; }
  void setMovie () { movie = true; movie_index = 0; }
  void setCurrentFieldRank (int r) { currentFieldRank = r; }

  glgrib_options_light light;
private:
  bool movie = false;
  int movie_index = 0;
  double movie_time = 0;
  void setLightShader (GLuint) const;
  int currentFieldRank = 0;
  glgrib_string str;
  glgrib_colorbar colorbar;
};

#endif
