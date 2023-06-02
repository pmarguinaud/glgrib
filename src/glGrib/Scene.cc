#include "glGrib/Scene.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Clear.h"

#include <sys/time.h>

namespace glGrib
{

void Scene::clear ()
{
  glGrib::clear (*this);
  Options opts;
  setup (opts);
}

Scene & Scene::operator= (const Scene & other)
{

  if (this != &other)
    {
      d = other.d;
     
      for (auto f : fieldlist)
        if (f != nullptr)
          delete f;
     
      fieldlist.clear ();
     
      for (auto f : other.fieldlist)
        if (f == nullptr)
          fieldlist.push_back (nullptr);
        else
          fieldlist.push_back (f->clone ());


      for (auto f : geopointslist)
        if (f != nullptr)
          delete f;
     
      geopointslist.clear ();
     
      for (auto f : other.geopointslist)
        if (f == nullptr)
          geopointslist.push_back (nullptr);
        else
          geopointslist.push_back (f->clone ());


   }
  return *this;
}

Scene::~Scene () 
{
  for (auto f : fieldlist)
    if (f != nullptr)
      delete f;
  for (auto f : geopointslist)
    if (f != nullptr)
      delete f;
}

void Scene::render (const Object3D * obj) const
{
  if (obj == nullptr)
    return;
  if (! obj->isReady ())
    return;
  if (! obj->visible ())
    return;
  obj->render (d.view, d.opts.scene.light);
}

void Scene::render (const Object2D * obj) const
{
  if (obj == nullptr)
    return;
  if (! obj->isReady ())
    return;
  obj->render (obj->getSide () == Object2D::LEFT ? d.MVP_L : d.MVP_R);
}

void Scene::render () const
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  d.test.render (d.view, d.opts.scene.light);
//d.vcut.render (d.view, d.opts.scene.light);
  

  std::vector<const Object3D*> obj_list;

  obj_list.push_back (&d.landscape);

  for (auto f : fieldlist)
    if (f != nullptr)
      obj_list.push_back (f);

  for (auto f : geopointslist)
    if (f != nullptr)
      obj_list.push_back (f);

  obj_list.push_back (&d.coast);
  obj_list.push_back (&d.border);
  obj_list.push_back (&d.rivers);
  obj_list.push_back (&d.departements);
  obj_list.push_back (&d.grid);
  obj_list.push_back (&d.cities);
  obj_list.push_back (&d.land);


  // Sort 3D objects : render farther first
  std::sort (obj_list.begin (), obj_list.end (), 
             [] (const Object3D * a, 
                 const Object3D * b) 
             { return a->getScale () < b->getScale (); });


  for (auto obj : obj_list)
    render (obj);

  render (&d.image);

  if (getFieldColorbar ())
    render (&d.colorbar);
  else if (getGeoPointsColorbar ()) 
    render (&d.colorbar);

  render (&d.mapscale);

  render (&d.strmess);

  render (&d.strdate);

  render (&d.strtitle);

  render (&d.ticks);

  for (auto & str : d.str)
    render (&str);

}

const OptionDate * Scene::getDate ()
{
  for (auto fld : fieldlist)
    if (fld)
      if (fld->isReady ())
        {
          const std::vector<FieldMetadata> & meta = fld->getMeta ();
	  if (meta.size () >= 1)
            return &meta[0].term;
        }
  return nullptr;
}


void Scene::updateLight ()
{
  if (d.opts.scene.light.rotate.on)
    d.opts.scene.light.lon -= d.opts.scene.light.rotate.rate;

  const OptionDate * date = nullptr;

  if (d.opts.scene.light.date_from_grib.on)
    date = getDate ();
  else if (d.opts.scene.light.date.year != 0)
    date = &d.opts.scene.light.date;

  if (date != nullptr)
    {
      const int nday[12] = {31, 28, 31, 30, 31, 30, 
                            31, 31, 30, 31, 30, 31};
      const float dtrop = 23.0f + 26.0f / 60.0f;  // Tropics
      const float eday  = 31 + 28 + 20;           // 20th of March, equinox
      float cday = date->day-1;
      float time = (date->hour * 60.0f + date->minute) * 60.0f + date->second;
      for (int m = 0; m < date->month-1; m++)
        cday += nday[m];
      d.opts.scene.light.lat = dtrop * std::sin (twopi * (cday - eday) / 365.0f);
      d.opts.scene.light.lon = 360.0f * ((12.0f * 3600.0f - time) / (24.0f * 3600.0f));
    }
}

namespace
{

float ffmod (float x, float y)
{
  while (x < 0)
    x += y;
  while (x > y)
    x -= y;
  return x;
}

}

void Scene::updateView ()
{
  OptionsView o = d.view.getOptions ();

  if (d.opts.scene.rotate_earth.on)
    o.lon += d.opts.scene.rotate_earth.rate;
  if (d.opts.scene.travelling.on)
    {
      float frames = d.opts.scene.travelling.frames;
      float a = static_cast<float> (d.nupdate) / frames;
      if (a <= 1)
        {
          float lon1 = d.opts.scene.travelling.pos1.lon;
          float lon2 = d.opts.scene.travelling.pos2.lon;
          float dlon21 = ffmod (lon2 - lon1, 360);
          float dlon12 = ffmod (lon1 - lon2, 360);

	  // Choose shortest path
	  float dlon = dlon21 > dlon12 ? -dlon12 : dlon21;

          float lat1 = d.opts.scene.travelling.pos1.lat;
          float lat2 = d.opts.scene.travelling.pos2.lat;
          float dlat = lat2 - lat1;

          float fov1 = d.opts.scene.travelling.pos1.fov;
          float fov2 = d.opts.scene.travelling.pos2.fov;
          float dfov = fov2 - fov1;

          o.lon = lon1 + dlon * a;
          o.lat = lat1 + dlat * a;
          o.fov = fov1 + dfov * a;

	  reSize ();
        }
    }
  else if ((d.opts.scene.lon_at_hour >= 0.0f) && 
           (d.opts.scene.lon_at_hour <= 24.0f))
    {
      const OptionDate * date = getDate ();
      if (date != nullptr)
        {
          float hh = d.opts.scene.lon_at_hour - (date->hour + date->minute / 60.0f);
	  if (hh > 24.0f) 
            hh -= 24.0f;
	  else if (hh < 0.0f)
            hh += 24.0f;
	  float lon = 360.0f * (hh / 24.0f);
	  if (lon < 0.0f)
            lon += 360.0f;
	  else if (lon > 360.0f)
            lon -= 360.0f;
          o.lon = lon;
        }
    }

  d.view.setOptions (o);

  d.ticks.reSize (d.view);
  d.mapscale.reSize (d.view);
}

void Scene::updateInterpolation ()
{
  if (d.opts.scene.interpolation.on)
    {
      int slotmax = 0;

      for (auto f : fieldlist)
        if (f != nullptr)
          slotmax = std::max (slotmax, f->getSlotMax ());

      slotmax--;

      float slot = static_cast<float> (d.nupdate) / static_cast<float> (d.opts.scene.interpolation.frames);
      if (slot > slotmax)
        slot = slotmax;

      for (size_t j = 0; j < fieldlist.size (); j++)
        setFieldOptions (j, d.opts.field[j], slot);
    }
}


void Scene::updateDate ()
{
  if (d.opts.scene.date.on)
    {
      const OptionDate * date = nullptr;

      Field * fld = getCurrentField ();
      if (fld != nullptr)
        {
          const std::vector<FieldMetadata> & meta = fld->getMeta ();
          date = &meta[0].term;
	}
      else
        {
          date = getDate ();
	}

      if (date != nullptr)
        {
          if (strdate != date->asString ())
            {
              strdate = date->asString ();
              d.strdate.update (strdate);
            }
	}
    }
}

std::string Scene::getCurrentFieldName () const
{
  std::string name;

  Field * fld = getCurrentField ();
  if (fld != nullptr) 
    {
      const std::vector<FieldMetadata> & meta = fld->getMeta ();
      name = meta[0].getName ();
    }
  return name;
}

void Scene::updateTitle ()
{
  if (d.opts.scene.title.on)
    {
      std::string title = d.opts.scene.title.text;
      std::string fname = getCurrentFieldName ();

      if ((fname != "") && (title == ""))
        title = fname;
      if (strtitle != title)
        {
          glGrib::clear (d.strtitle);
          FontPtr font = getGlGribFontPtr (d.opts.scene.title.font);
          d.strtitle.setup (font, title, d.opts.scene.title.x, 
                              d.opts.scene.title.y, d.opts.scene.title.font.bitmap.scale, 
                              StringTypes::str2align (d.opts.scene.title.a));
          d.strtitle.setForegroundColor (d.opts.scene.title.font.color.foreground);
          d.strtitle.setBackgroundColor (d.opts.scene.title.font.color.background);
        }
    }
}

void Scene::updateColorbar ()
{
  const Field * fld = getFieldColorbar ();
  const GeoPoints * points = getGeoPointsColorbar ();
  if (fld)
    d.colorbar.update (fld->getPalette ());
  else if (points)
    d.colorbar.update (points->getPalette ());
}

void Scene::updateGeoPoints ()
{
  for (auto points : geopointslist)
    if (points != nullptr)
      points->update ();
}

void Scene::updateFields ()
{
  for (auto fld : fieldlist)
    if (fld != nullptr)
      fld->update ();
}

void Scene::update ()
{
  updateColorbar ();
  updateView ();
  updateLight ();
  updateInterpolation ();
  updateDate ();
  updateTitle ();
  updateGeoPoints ();
  updateFields ();

  d.nupdate++;

}

void Scene::setup (const Options & o)
{
  d.opts = o;

  d.test.setup ();
//d.vcut.setup ();

  setViewport (d.opts.render.width, d.opts.render.height);
  setViewOptions (d.opts.view);
  setLightOptions (d.opts.scene.light);

  setObjectOptions (d.image, d.opts.scene.image);

  if (d.opts.scene.interpolation.on)
    {
      int size = 0;
      for (auto f : d.opts.field)
        {
          bool defined = f.path.size () != 0;
	  if (! defined)
            continue;
          size += 4;
	}
      ld.setSize (size);
    }
  else
    {
      int size = 0;
      for (auto f : d.opts.field)
        if (f.path.size () != 0)
          size++;
      ld.setSize (size);
    }

  setLandscapeOptions (d.opts.landscape);
  setGridOptions (d.opts.grid);
  setTicksOptions (d.opts.ticks);
  setLandOptions (d.opts.land);
  setCoastOptions (d.opts.coast);
  setBorderOptions (d.opts.border);
  setRiversOptions (d.opts.rivers);
  setDepartementsOptions (d.opts.departements);

  for (auto f : d.opts.field)
    fieldlist.push_back ((Field *)nullptr);

  for (size_t i = 0; i < d.opts.field.size (); i++)
    setFieldOptions (i, d.opts.field[i]);

  setDateOptions (d.opts.scene.date);
  setTitleOptions (d.opts.scene.title);
  setTextOptions (d.opts.scene.text);
  setCitiesOptions (d.opts.cities);

  for (auto p : d.opts.geopoints)
    geopointslist.push_back ((GeoPoints *)nullptr);

  for (size_t i = 0; i < d.opts.geopoints.size (); i++)
    setGeoPointsOptions (i, d.opts.geopoints[i]);

  setMapScaleOptions (d.opts.mapscale);
  setColorBarOptions (d.opts.colorbar);

  d.currentFieldRank = d.opts.scene.select.field_rank;


  if (d.opts.scene.center.on)
    centerOnCurrentField ();

  reSize ();
}

void Scene::centerOnCurrentField ()
{
  Field * field = getCurrentField ();
  if (field != nullptr)
    {
      const_GeometryPtr geometry = field->getGeometry ();
      if (d.opts.scene.center.gridpoint < 0)
        {
          geometry->getView (&d.view);
        }
      else
        {
          OptionsView view_opts = d.view.getOptions ();
          float lon, lat;
          geometry->index2latlon (d.opts.scene.center.gridpoint, &lat, &lon);
          view_opts.lat = lat * rad2deg;
          view_opts.lon = lon * rad2deg;
          d.view.setOptions (view_opts);
        }
    }
}

void Scene::setViewport (int _width, int _height)
{
  d.view.setViewport (_width, _height);
  float width = d.view.getWidth (), height = d.view.getHeight ();
  float ratio = width / height;

  d.MVP_L = glm::ortho (0.0f, ratio, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  
  d.MVP_R = glm::ortho (1.0f - ratio, 1.0f, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
}

void Scene::reSize ()
{
  d.landscape.reSize (d.view);
  d.coast.reSize (d.view);
  d.border.reSize (d.view);
  d.rivers.reSize (d.view);
  d.departements.reSize (d.view);
  d.grid.reSize (d.view);
  d.ticks.reSize (d.view);
  d.mapscale.reSize (d.view);
  for (auto f : fieldlist)
    if (f)
      f->reSize (d.view);
}

const Options Scene::getOptions () const
{
  Options o = d.opts;

  o.view           = d.view.getOptions ();
  o.landscape      = d.landscape.getOptions ();
  o.grid           = d.grid.getOptions ();
  o.ticks          = d.ticks.getOptions ();
  o.coast          = d.coast.getOptions ();
  o.border         = d.border.getOptions ();
  o.rivers         = d.rivers.getOptions ();
  o.departements   = d.departements.getOptions ();
  o.cities         = d.cities.getOptions ();
  o.mapscale       = d.mapscale.getOptions ();
  o.land           = d.land.getOptions ();

  for (size_t i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != nullptr)
      o.field[i] = fieldlist[i]->getOptions ();

  for (size_t i = 0; i < geopointslist.size (); i++)
    if (geopointslist[i] != nullptr)
      o.geopoints[i] = geopointslist[i]->getOptions ();

  return o;
}

void Scene::setViewOptions (const OptionsView & o)
{
  d.view.setup (o);
  reSize ();
}

void Scene::setLandscapeOptions (const OptionsLandscape & o)
{
  glGrib::clear (d.landscape);
  d.landscape.setup (&ld, o);
}

void Scene::setGeoPointsOptions (int j, const OptionsGeoPoints & o)
{
  if (geopointslist[j] != nullptr)
    delete geopointslist[j];

  geopointslist[j] = new GeoPoints ();
  geopointslist[j]->setup (o);
}

void Scene::setFieldOptions (int j, const OptionsField & o, float slot)
{
  if (fieldlist[j] != nullptr)
    delete fieldlist[j];

  fieldlist[j] = Field::create (o, slot, &ld);

  if (d.opts.scene.center.on)
    centerOnCurrentField ();
}

void Scene::setColorBarOptions (const OptionsColorbar & o)
{
  d.opts.colorbar = o;
  glGrib::clear (d.strmess);
  glGrib::clear (d.colorbar);

  if (d.opts.colorbar.on)
    {
      FontPtr font = getGlGribFontPtr (d.opts.colorbar.font);
      d.strmess.setup (font, std::string (30, ' '), 1.0f, 1.0f, 
                       d.opts.colorbar.font.bitmap.scale, StringTypes::NE);
      d.strmess.setForegroundColor (d.opts.colorbar.font.color.foreground);
      d.strmess.setSide (Object2D::RIGHT);
      d.colorbar.setup (d.opts.colorbar);
      updateColorbar ();
    }
}

void Scene::setTextOptions (const OptionsText & o)
{
  d.opts.scene.text = o;
  glGrib::clear (d.str);
  if (d.opts.scene.text.on)
    {
      FontPtr font = getGlGribFontPtr (d.opts.scene.text.font);
      for (size_t i = 0; i < d.opts.scene.text.s.size (); i++)
        {
          String2D<0,1> str;
          d.str.push_back (str);

          if (i >= d.opts.scene.text.x.size ())
            break;
          if (i >= d.opts.scene.text.y.size ())
            break;

          StringTypes::align_t a = i < d.opts.scene.text.a.size () ? 
            StringTypes::str2align (d.opts.scene.text.a[i]) : StringTypes::C;

          d.str[i].setup (font, d.opts.scene.text.s[i], d.opts.scene.text.x[i], 
                          d.opts.scene.text.y[i], d.opts.scene.text.font.bitmap.scale, a);
          d.str[i].setForegroundColor (d.opts.scene.text.font.color.foreground);
          d.str[i].setBackgroundColor (d.opts.scene.text.font.color.background);
          d.str[i].setSide (Object2D::RIGHT);
        }
    }
}

void Scene::setGridColorOptions (const OptionColor & color)
{
  d.grid.setColorOptions (color);
}

void Scene::setGridScaleOptions (float scale)
{
  d.grid.setScaleOptions (scale);
}

void Scene::setFieldPaletteOptions (int j, const OptionsPalette & opts)
{
  Field * fld = fieldlist[j];
  if (fld == nullptr)
    return;
  fld->setPaletteOptions (opts);
}

void Scene::setDateOptions (const OptionsDate & o)
{
  strdate = "";
  d.opts.scene.date = o;
  glGrib::clear (d.strdate);
  if (d.opts.scene.date.on)
    {
      FontPtr font = getGlGribFontPtr (d.opts.scene.date.font);
      d.strdate.setup (font, std::string (20, ' '), 1.0f, 0.0f, 
                       d.opts.scene.date.font.bitmap.scale, StringTypes::SE);
      d.strdate.setSide (Object2D::RIGHT);
      d.strdate.setForegroundColor (d.opts.scene.date.font.color.foreground);
      d.strdate.setBackgroundColor (d.opts.scene.date.font.color.background);
    }
}

void Scene::setTitleOptions (const OptionsTitle & o)
{
  strtitle = "";
  d.opts.scene.title = o;
  glGrib::clear (d.strtitle);
  if (d.opts.scene.title.on)
    {
    }
}

void Scene::setLightOptions (const OptionsLight & o)
{
  d.opts.scene.light = o;
}

void Scene::setSceneOptions (const OptionsScene & o)
{
  d.opts.scene.rotate_earth = o.rotate_earth;
  d.opts.scene.lon_at_hour  = o.lon_at_hour;
}

}
