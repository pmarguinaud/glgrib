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
  glgrib_scene & operator= (const glgrib_scene & other);
  virtual ~glgrib_scene ();
  void init (const glgrib_options &);
  void display () const;
  void display_obj (const glgrib_object *) const;

  glgrib_field * getCurrentField () 
    { 
      return d.currentFieldRank < fieldlist.size () ? fieldlist[d.currentFieldRank] : NULL; 
    }
  glgrib_field_display_options * getCurrentFieldOpts () 
    { 
      if (d.currentFieldRank < fieldlist.size ())
        if (fieldlist[d.currentFieldRank] != NULL)
          return &fieldlist[d.currentFieldRank]->dopts;
      return NULL;
    }

  void getLightPos (float * lon, float * lat) const
    {
      *lon = d.light.lon;
      *lat = d.light.lat;
    }
  void setLightPos (float lon, float lat) 
    { 
      d.light.lon = lon;
      d.light.lat = lat;
    }
  void setLight ()
    {
      d.light.on = true;
    }
  void unsetLight () { d.light.on = false; }
  bool hasLight () const { return d.light.on; }
  void update ();
  void toggleMovie () { d.movie = ! d.movie; d.movie_index = 0; }
  void setMovie () { d.movie = true; d.movie_index = 0; }
  void setCurrentFieldRank (int r) { d.currentFieldRank = r; }

  void setViewport (int, int);

  void setMessage (const std::string & mess) { d.str.update (mess); }
  void resize ();

  std::vector<glgrib_field*> fieldlist;


  class _data
  {
    public:
      glgrib_options_light light;
      glgrib_options opts;
      glgrib_view view;
      glgrib_landscape landscape;
      glgrib_coastlines coastlines;
      glgrib_grid grid;
      bool rotate_earth = false;
    private:
      glgrib_colorbar colorbar;
      glgrib_string str;
      glgrib_string strxyz;
      bool movie = false;
      int movie_index = 0;
      int nupdate = 0;
      double movie_time = 0;
      int currentFieldRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class glgrib_scene;
  };

  _data d;


};

#endif
