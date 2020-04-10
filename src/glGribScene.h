#pragma once

#include <vector>
#include <set>

#include "glGribFont.h"
#include "glGribImage.h"
#include "glGribString.h"
#include "glGribView.h"
#include "glGribObject.h"
#include "glGribProgram.h"
#include "glGribLandScape.h"
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

namespace glGrib
{

class Scene
{
public:


  Scene () {}
  Scene & operator= (const Scene & other);
  virtual ~Scene ();
  void setup (const Options &);
  void display () const;
  void displayObj (const Object *) const;

  int getCurrentFieldRank () const
  {
    return d.currentFieldRank;
  }

  Field * getCurrentField () 
  { 
    return static_cast<size_t> (d.currentFieldRank) < fieldlist.size () ? fieldlist[d.currentFieldRank] : nullptr; 
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
  void updateLight ();
  void updateInterpolation ();
  void updateView ();
  void updateDate ();
  void updateTitle ();

  void setCurrentFieldRank (int r) { d.currentFieldRank = r; }

  void setViewport (int, int);

  void setMessage (const std::string & mess) { if (d.strmess.isReady ()) d.strmess.update (mess); }
  void reSize ();
  const OptionDate * getDate ();

  std::vector<Field*> fieldlist;


  class _data
  {
    public:
      Options opts;
      View view;
      Landscape landscape;
      Coast coast;
      Border border;
      Rivers rivers;
      Grid grid;
      Ticks ticks;
      Departements departements;
      Cities cities;
      Test test;
      Land land;
    private:
      Image image;
      Colorbar colorbar;
      Mapscale mapscale;
      String strmess;
      String strdate;
      String strtitle;
      std::vector<String> str;
      int nupdate = 0;
      int currentFieldRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class Scene;
  };
  std::string strdate = "";
  std::string strtitle = "";

  void toggleColorBar ()
  {
    d.colorbar.toggleHidden ();
  }

  _data d;

  Loader ld;

  void setViewOptions (const OptionsView &);
  void setLandscapeOptions (const OptionsLandscape &);
  void setGridOptions (const OptionsGrid &);
  void setTicksOptions (const OptionsTicks &);
  void setLandOptions (const OptionsLand &);
  void setGridColorOptions (const OptionColor &);
  void setGridScaleOptions (float);
  void setCoastOptions (const OptionsCoast &);
  void setBorderOptions (const OptionsBorder &);
  void setRiversOptions (const OptionsRivers &);
  void setDepartementsOptions (const OptionsDepartements &);
  void setFieldOptions (int, const OptionsField &, float = 0);
  void setFieldPaletteOptions (int, const OptionsPalette &);
  void setColorBarOptions (const OptionsColorbar &);
  void setMapScaleOptions (const OptionsMapscale &);
  void setImageOptions (const OptionsImage &);
  void setTextOptions (const OptionsText &);
  void setDateOptions (const OptionsDate &);
  void setLightOptions (const OptionsLight &);
  void setSceneOptions (const OptionsScene &);
  void setCitiesOptions (const OptionsCities &);
  void setTitleOptions (const OptionsTitle &);

  Options getOptions () const;
  const OptionsScene & getSceneOptions () const { return d.opts.scene; }

};


}
