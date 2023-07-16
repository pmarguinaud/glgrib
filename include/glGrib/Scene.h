#pragma once

#include <vector>
#include <set>

#include "glGrib/Font.h"
#include "glGrib/Image.h"
#include "glGrib/String.h"
#include "glGrib/View.h"
#include "glGrib/Object.h"
#include "glGrib/Program.h"
#include "glGrib/LandScape.h"
#include "glGrib/Grid.h"
#include "glGrib/Ticks.h"
#include "glGrib/Field.h"
#include "glGrib/Coast.h"
#include "glGrib/Border.h"
#include "glGrib/Rivers.h"
#include "glGrib/Departements.h"
#include "glGrib/Colorbar.h"
#include "glGrib/Mapscale.h"
#include "glGrib/Cities.h"
#include "glGrib/GeoPoints.h"
#include "glGrib/Test.h"
#include "glGrib/Land.h"
#include "glGrib/Loader.h"
#include "glGrib/Clear.h"

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

  int getCurrentGeoPointsRank () const
  {
    return d.currentGeoPointsRank;
  }

  Field * getCurrentField () const
  { 
    size_t k = static_cast<size_t> (d.currentFieldRank);
    return (0 <= k) && (k < fieldlist.size ()) ? fieldlist[d.currentFieldRank] : nullptr; 
  }

  GeoPoints * getCurrentGeoPoints () const
  {
    size_t k = static_cast<size_t> (d.currentGeoPointsRank);
    return (0 <= k) && (k < geopointslist.size ()) ? geopointslist[d.currentGeoPointsRank] : nullptr; 
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
  void updateFields ();

  void setCurrentFieldRank (int r) 
  { 
    d.currentFieldRank = r; 
    d.currentGeoPointsRank = -1;
  }
  void setCurrentGeoPointsRank (int r) 
  { 
    d.currentGeoPointsRank = r; 
    d.currentFieldRank = -1;
  }

  const glGrib::Field * getFieldColorbar () const
  {
    const glGrib::Field * fld = getCurrentField ();
    if ((fld != nullptr) && (d.colorbar.isReady ()))
      if (fld->useColorBar ())
        return fld;
    return nullptr;
  }

  const glGrib::GeoPoints * getGeoPointsColorbar () const
  {
    const glGrib::GeoPoints * points = getCurrentGeoPoints ();
    if ((points != nullptr) && (d.colorbar.isReady ()))
      if (points->useColorBar ())
        return points;
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
  void setGeoPointsOptions (int, const OptionsGeoPoints &);

  void setFieldOptions (int, const OptionsField &);
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
  std::vector<GeoPoints*> geopointslist;

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
      String2D<0,1> strmess;
      String2D<0,1> strdate;
      String2D<0,1> strtitle;
      std::vector<String2D<0,1>> str;
      int nupdate = 0;
      int currentFieldRank = 0;
      int currentGeoPointsRank = 0;
      glm::mat4 MVP_R, MVP_L;
      friend class Scene;
  };
  std::string strdate = "";
  std::string strtitle = "";

  _data d;

  Loader ld;

};


}
