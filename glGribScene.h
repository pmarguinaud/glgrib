#pragma once

#include <vector>
#include <set>

#include "glGribFont.h"
#include "glGribImage.h"
#include "glGribString.h"
#include "glGribView.h"
#include "glGribObject.h"
#include "glGribProgram.h"
#include "glGribLandscape.h"
#include "glGribGrid.h"
#include "glGribTicks.h"
#include "glGribField.h"
#include "glGribCoast.h"
#include "glGribBorder.h"
#include "glGribRivers.h"
#include "glGribDepartements.h"
#include "glGribColorbar.h"
#include "glGribMapscale.h"
#include "glGribCities.h"
#include "glGribTest.h"
#include "glGribLand.h"
#include "glGribLoader.h"

#include <set>

class glGribScene
{
public:


  glGribScene () {}
  glGribScene & operator= (const glGribScene & other);
  virtual ~glGribScene ();
  void setup (const glGribOptions &);
  void display () const;
  void displayObj (const glGribObject *) const;

  glGribField * getCurrentField () 
    { 
      return d.currentFieldRank < fieldlist.size () ? fieldlist[d.currentFieldRank] : nullptr; 
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

  void setMessage (const std::string & mess) { if (d.strmess.isReady ()) d.strmess.update (mess); }
  void resize ();
  const glGribOptionDate * getDate ();

  std::vector<glGribField*> fieldlist;


  class _data
  {
    public:
      glGribOptions opts;
      glGribView view;
      glGribLandscape landscape;
      glGribCoast coast;
      glGribBorder border;
      glGribRivers rivers;
      glGribGrid grid;
      glGribTicks ticks;
      glGribDepartements departements;
      glGribCities cities;
      glGribTest test;
      glGribLand land;
    private:
      glGribImage image;
      glGribColorbar colorbar;
      glGribMapscale mapscale;
      glGribString strmess;
      glGribString strdate;
      glGribString strtitle;
      std::vector<glGribString> str;
      int nupdate = 0;
      int currentFieldRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class glGribScene;
  };
  std::string strdate = "";
  std::string strtitle = "";

  void toggleColorBar ()
  {
    d.colorbar.toggleHidden ();
  }

  _data d;

  glGribLoader ld;

  void setViewOptions (const glGribOptionsView &);
  void setLandscapeOptions (const glGribOptionsLandscape &);
  void setGridOptions (const glGribOptionsGrid &);
  void setTicksOptions (const glGribOptionsTicks &);
  void setLandOptions (const glGribOptionsLand &);
  void setGridColorOptions (const glGribOptionColor &);
  void setGridScaleOptions (float);
  void setCoastOptions (const glGribOptionsCoast &);
  void setBorderOptions (const glGribOptionsBorder &);
  void setRiversOptions (const glGribOptionsRivers &);
  void setDepartementsOptions (const glGribOptionsDepartements &);
  void setFieldOptions (int, const glGribOptionsField &, float = 0);
  void setFieldPaletteOptions (int, const glGribOptionsPalette &);
  void setColorBarOptions (const glGribOptionsColorbar &);
  void setMapScaleOptions (const glGribOptionsMapscale &);
  void setImageOptions (const glGribOptionsImage &);
  void setTextOptions (const glGribOptionsText &);
  void setDateOptions (const glGribOptionsDate &);
  void setLightOptions (const glGribOptionsLight &);
  void setSceneOptions (const glGribOptionsScene &);
  void setCitiesOptions (const glGribOptionsCities &);
  void setTitleOptions (const glGribOptionsTitle &);

  glGribOptions getOptions () const;
  const glGribOptionsScene & getSceneOptions () const { return d.opts.scene; }

};

