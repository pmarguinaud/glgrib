#ifndef _GLGRIB_SCENE_H
#define _GLGRIB_SCENE_H

#include <vector>
#include <set>

#include "glgrib_font.h"
#include "glgrib_image.h"
#include "glgrib_string.h"
#include "glgrib_view.h"
#include "glgrib_object.h"
#include "glgrib_program.h"
#include "glgrib_landscape.h"
#include "glgrib_grid.h"
#include "glgrib_field.h"
#include "glgrib_coast.h"
#include "glgrib_border.h"
#include "glgrib_rivers.h"
#include "glgrib_departements.h"
#include "glgrib_colorbar.h"
#include "glgrib_mapscale.h"
#include "glgrib_cities.h"
#include "glgrib_test.h"
#include "glgrib_loader.h"

#include <set>

class glgrib_scene
{
public:


  glgrib_scene () {}
  glgrib_scene & operator= (const glgrib_scene & other);
  virtual ~glgrib_scene ();
  void setup (const glgrib_options &);
  void display () const;
  void display_obj (const glgrib_object *) const;

  glgrib_field * getCurrentField () 
    { 
      return d.currentFieldRank < fieldlist.size () ? fieldlist[d.currentFieldRank] : NULL; 
    }

  void getLightPos (float * lon, float * lat) const
    {
      *lon = d.opts.scene.light.lon;
      *lat = d.opts.scene.light.lat;
    }
  void setLightPos (float lon, float lat) 
    { 
      d.opts.scene.light.lon = lon;
      d.opts.scene.light.lat = lat;
    }
  void setLight ()
    {
      d.opts.scene.light.on = true;
    }
  void unsetLight () { d.opts.scene.light.on = false; }
  bool hasLight () const { return d.opts.scene.light.on; }
  void update ();
  void update_light ();
  void update_interpolation ();
  void update_view ();
  void update_date ();
  void update_title ();

  void setCurrentFieldRank (int r) { d.currentFieldRank = r; }

  void setViewport (int, int);

  void setMessage (const std::string & mess) { d.strmess.update (mess); }
  void resize ();
  const glgrib_option_date * get_date ();

  std::vector<glgrib_field*> fieldlist;


  class _data
  {
    public:
      glgrib_options opts;
      glgrib_view view;
      glgrib_landscape landscape;
      glgrib_coast coast;
      glgrib_border border;
      glgrib_rivers rivers;
      glgrib_grid grid;
      glgrib_departements departements;
      glgrib_cities cities;
      glgrib_test test;
    private:
      glgrib_image image;
      glgrib_colorbar colorbar;
      glgrib_mapscale mapscale;
      glgrib_string strmess;
      glgrib_string strdate;
      glgrib_string strxyz;
      glgrib_string strtitle;
      std::vector<glgrib_string> str;
      int nupdate = 0;
      int currentFieldRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class glgrib_scene;
  };
  std::string strdate = "";
  std::string strtitle = "";

  void toggleColorBar ()
  {
    d.colorbar.toggleHidden ();
  }

  _data d;

  glgrib_loader ld;

  void setViewOptions (const glgrib_options_view &);
  void setLandscapeOptions (const glgrib_options_landscape &);
  void setGridOptions (const glgrib_options_grid &);
  void setGridColorOptions (const glgrib_option_color &);
  void setGridScaleOptions (float);
  void setCoastOptions (const glgrib_options_coast &);
  void setBorderOptions (const glgrib_options_border &);
  void setRiversOptions (const glgrib_options_rivers &);
  void setDepartementsOptions (const glgrib_options_departements &);
  void setFieldOptions (int, const glgrib_options_field &, float = 0);
  void setFieldPaletteOptions (int, const glgrib_options_palette &);
  void setColorBarOptions (const glgrib_options_colorbar &);
  void setMapScaleOptions (const glgrib_options_mapscale &);
  void setImageOptions (const glgrib_options_image &);
  void setTextOptions (const glgrib_options_text &);
  void setDateOptions (const glgrib_options_date &);
  void setLightOptions (const glgrib_options_light &);
  void setSceneOptions (const glgrib_options_scene &);
  void setCitiesOptions (const glgrib_options_cities &);
  void setTitleOptions (const glgrib_options_title &);

  glgrib_options getOptions () const;
  const glgrib_options_scene & getSceneOptions () const { return d.opts.scene; }

};

#endif
