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
#include "glGribGeoPoints.h"
#include "glGribTest.h"
#include "glGribLand.h"
#include "glGribLoader.h"
#include "glGribClear.h"

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
  void render () const;
  void render (const Object3D *) const;
  void render (const Object2D *) const;
 
  void clear ();

  int getCurrentFieldRank () const
  {
    return d.currentFieldRank;
  }

  Field * getCurrentField () const
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
  void updateColorbar ();
  void updateLight ();
  void updateInterpolation ();
  void updateView ();
  void updateDate ();
  void updateTitle ();
  void updateGeoPoints ();

  void setCurrentFieldRank (int r) { d.currentFieldRank = r; }

  const glGrib::Field * getFieldColorbar () const
  {
    const glGrib::Field * fld = getCurrentField ();
    if ((fld != nullptr) && (d.colorbar.isReady ()))
      if (fld->useColorBar ())
        return fld;
    return nullptr;
  }

  void setViewport (int, int);

  void setMessage (const std::string & mess) { if (d.strmess.isReady ()) d.strmess.update (mess); }
  void reSize ();
  const OptionDate * getDate ();

  template <typename T, typename O>
  void setObjectOptions (T & object, const O & o)
  {
    glGrib::clear (object);
    object.setup (o);
    object.reSize (d.view);
  }

  void hideAllFields ()
  {
    for (auto f : fieldlist)
      if (f != nullptr)
        f->hide ();
  }
  
  void showAllFields ()
  {
    for (auto f : fieldlist)
      if (f != nullptr)
        f->show ();
  }

  const View & getView () const
  {
    return d.view;
  }

  View & getView () 
  {
    return d.view;
  }

  void setViewOptions (const OptionsView &);
  void setLandscapeOptions (const OptionsLandscape &);
  void setLandscapeWireFrameOption (bool wireframe)
  {
    d.landscape.setWireFrameOption (wireframe);
  }
  void setLandscapeFlatOption (bool flat)
  {
    d.landscape.setFlatOption (flat);
  }
  void setLandscapePositionOptions (const glGrib::OptionsLandscapePosition &o)
  {
    d.landscape.setPositionOptions (o);
  }
  void setGridOptions (const OptionsGrid & o)
  {
    setObjectOptions (d.grid, o);
  }
  void setTicksOptions (const OptionsTicks & o)
  {
    setObjectOptions (d.ticks, o);
  }
  void setLandOptions (const OptionsLand & o)
  {
    setObjectOptions (d.land, o);
  }
  void setCoastOptions (const OptionsCoast & o)
  {
    setObjectOptions (d.coast, o);
  }
  void setBorderOptions (const OptionsBorder & o)
  {
    setObjectOptions (d.border, o);
  }
  void setRiversOptions (const OptionsRivers & o)
  {
    setObjectOptions (d.rivers, o);
  }
  void setDepartementsOptions (const OptionsDepartements & o)
  {
    setObjectOptions (d.departements, o);
  }
  void setMapScaleOptions (const OptionsMapscale & o)
  {
    setObjectOptions (d.mapscale, o);
  }
  void setImageOptions (const OptionsImage & o)
  {
    setObjectOptions (d.image, o);
  }
  void setCitiesOptions (const OptionsCities & o)
  {
    setObjectOptions (d.cities, o);
  }
  void setGeoPointsOptions (const OptionsGeoPoints & o)
  {
    setObjectOptions (d.geopoints, o);
  }

  void setFieldOptions (int, const OptionsField &, float = 0);
  void setFieldPaletteOptions (int, const OptionsPalette &);
  void setGridColorOptions (const OptionColor &);
  void setGridScaleOptions (float);
  void setColorBarOptions (const OptionsColorbar &);
  void setTextOptions (const OptionsText &);
  void setDateOptions (const OptionsDate &);
  void setLightOptions (const OptionsLight &);
  void setSceneOptions (const OptionsScene &);
  void setTitleOptions (const OptionsTitle &);

  const Options getOptions () const;

  const OptionsScene & getSceneOptions () const 
  { 
    return d.opts.scene; 
  }
  const OptionsView & getViewOptions () const
  {
    return d.view.getOptions ();
  }
  const OptionsLandscape & getLandscapeOptions () const
  {
    return d.landscape.getOptions ();
  }
  const OptionsGrid & getGridOptions () const
  {
    return d.grid.getOptions ();
  }
  const OptionsTicks & getTicksOptions () const
  {
    return d.ticks.getOptions ();
  }
  const OptionsLand & getLandOptions () const
  {
    return d.land.getOptions ();
  }
  const OptionsCoast & getCoastOptions () const
  {
    return d.coast.getOptions ();
  }
  const OptionsBorder & getBorderOptions () const
  {
    return d.border.getOptions ();
  }
  const OptionsRivers & getRiversOptions () const
  {
    return d.rivers.getOptions ();
  }
  const OptionsDepartements & getDepartementsOptions () const
  {
    return d.departements.getOptions ();
  }
  const OptionsMapscale & getMapScaleOptions () const
  {
    return d.mapscale.getOptions ();
  }
  const OptionsImage & getImageOptions () const
  {
    return d.image.getOptions ();
  }
  const OptionsCities & getCitiesOptions () const
  {
    return d.cities.getOptions ();
  }
  const OptionsGeoPoints & getGeoPointsOptions () const
  {
    return d.geopoints.getOptions ();
  }

  const OptionColor & getGridColorOptions () const;
  const OptionsColorbar & getColorBarOptions () const;
  const OptionsText & getTextOptions () const;
  const OptionsDate & getDateOptions () const;
  const OptionsLight & getLightOptions () const;
  const OptionsTitle & getTitleOptions () const;

  std::string getCurrentFieldName () const;

  void centerOnCurrentField ();

private:
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
      GeoPoints geopoints;
      Test test;
      Land land;
    private:
      Image image;
      Colorbar colorbar;
      Mapscale mapscale;
      String2D<0,1> strmess;
      String2D<0,1> strdate;
      String2D<0,1> strtitle;
      std::vector<String2D<0,1>> str;
      int nupdate = 0;
      int currentFieldRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class Scene;
  };
  std::string strdate = "";
  std::string strtitle = "";

  _data d;

  Loader ld;

};


}
