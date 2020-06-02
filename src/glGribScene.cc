#include "glGribScene.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"
#include "glGribClear.h"

#include <stdio.h>
#include <sys/time.h>


glGrib::Scene & glGrib::Scene::operator= (const glGrib::Scene & other)
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
   }
  return *this;
}

glGrib::Scene::~Scene () 
{
  for (auto f : fieldlist)
    if (f != nullptr)
      delete f;
}

void glGrib::Scene::render (const glGrib::Object3D * obj) const
{
  if (obj == nullptr)
    return;
  if (! obj->isReady ())
    return;
  if (! obj->visible ())
    return;
  obj->render (d.view, d.opts.scene.light);
}

void glGrib::Scene::render (const glGrib::Object2D * obj) const
{
  if (obj == nullptr)
    return;
  if (! obj->isReady ())
    return;
  obj->render (obj->getSide () == Object2D::LEFT ? d.MVP_L : d.MVP_R);
}

void glGrib::Scene::render () const
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  d.test.render (d.view, d.opts.scene.light);
  

  std::vector<const glGrib::Object3D*> obj_list;

  obj_list.push_back (&d.landscape);

  for (auto f : fieldlist)
    if (f != nullptr)
      obj_list.push_back (f);

  obj_list.push_back (&d.coast);
  obj_list.push_back (&d.border);
  obj_list.push_back (&d.rivers);
  obj_list.push_back (&d.departements);
  obj_list.push_back (&d.grid);
  obj_list.push_back (&d.cities);
  obj_list.push_back (&d.geopoints);
  obj_list.push_back (&d.land);


  // Sort 3D objects : render farther first
  std::sort (obj_list.begin (), obj_list.end (), 
             [] (const glGrib::Object3D * a, 
                 const glGrib::Object3D * b) 
             { return a->getScale () < b->getScale (); });


  for (auto obj : obj_list)
    render (obj);

  render (&d.image);

  if (getFieldColorbar ())
    render (&d.colorbar);
  else if (d.geopoints.isReady ())
    render (&d.colorbar);

  render (&d.mapscale);

  render (&d.strmess);

  render (&d.strdate);

  render (&d.strtitle);

  render (&d.ticks);

  for (auto & str : d.str)
    render (&str);

}

const glGrib::OptionDate * glGrib::Scene::getDate ()
{
  for (auto fld : fieldlist)
    if (fld)
      if (fld->isReady ())
        {
          const std::vector<glGrib::FieldMetadata> & meta = fld->getMeta ();
	  if (meta.size () >= 1)
            return &meta[0].term;
        }
  return nullptr;
}


void glGrib::Scene::updateLight ()
{
  if (d.opts.scene.light.rotate.on)
    d.opts.scene.light.lon -= d.opts.scene.light.rotate.rate;

  const glGrib::OptionDate * date = nullptr;

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
      d.opts.scene.light.lat = dtrop * sin (twopi * (cday - eday) / 365.0f);
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

void glGrib::Scene::updateView ()
{
  glGrib::OptionsView o = d.view.getOptions ();

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
      const glGrib::OptionDate * date = getDate ();
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

void glGrib::Scene::updateInterpolation ()
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


void glGrib::Scene::updateDate ()
{
  if (d.opts.scene.date.on)
    {
      const glGrib::OptionDate * date = nullptr;

      glGrib::Field * fld = getCurrentField ();
      if (fld != nullptr)
        {
          const std::vector<glGrib::FieldMetadata> & meta = fld->getMeta ();
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

void glGrib::Scene::updateTitle ()
{
  if (d.opts.scene.title.on)
    {
      std::string title = d.opts.scene.title.text;
      glGrib::Field * fld = getCurrentField ();
      if ((fld != nullptr) && (title == ""))
        {
          const std::vector<glGrib::FieldMetadata> & meta = fld->getMeta ();
          title = meta[0].getName ();
	}
      if (strtitle != title)
        {
          clear (d.strtitle);
          glGrib::FontPtr font = getGlGribFontPtr (d.opts.scene.title.font);
          d.strtitle.setup (font, title, d.opts.scene.title.x, 
                              d.opts.scene.title.y, d.opts.scene.title.font.scale, 
                              glGrib::String::str2align (d.opts.scene.title.a));
          d.strtitle.setForegroundColor (d.opts.scene.title.font.color.foreground);
          d.strtitle.setBackgroundColor (d.opts.scene.title.font.color.background);
        }
    }
}

void glGrib::Scene::updateColorbar ()
{
  const glGrib::Field * fld = getFieldColorbar ();
  if (fld)
    d.colorbar.update (fld->getPalette ());
  else if (d.geopoints.isReady ())
    d.colorbar.update (d.geopoints.getPalette ());
}

void glGrib::Scene::updateGeoPoints ()
{
  d.geopoints.update ();
}

void glGrib::Scene::update ()
{
  updateColorbar ();
  updateView ();
  updateLight ();
  updateInterpolation ();
  updateDate ();
  updateTitle ();
  updateGeoPoints ();

  d.nupdate++;

}

void glGrib::Scene::setup (const glGrib::Options & o)
{
  d.opts = o;

  d.test.setup ();

  setViewport (d.opts.window.width, d.opts.window.height);
  setViewOptions (d.opts.view);
  setLightOptions (d.opts.scene.light);
  setImageOptions (d.opts.scene.image);

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
    fieldlist.push_back ((glGrib::Field *)nullptr);

  for (size_t i = 0; i < d.opts.field.size (); i++)
    setFieldOptions (i, d.opts.field[i]);

  setDateOptions (d.opts.scene.date);
  setTitleOptions (d.opts.scene.title);
  setTextOptions (d.opts.scene.text);
  setCitiesOptions (d.opts.cities);
  setGeoPointsOptions (d.opts.geopoints);
  setColorBarOptions (d.opts.colorbar);
  setMapScaleOptions (d.opts.mapscale);

  d.currentFieldRank = d.opts.scene.select.field;


  if (d.opts.scene.center.on)
    {
      glGrib::Field * field = getCurrentField ();
      if (field != nullptr)
        {
          glGrib::const_GeometryPtr geometry = field->getGeometry ();
          geometry->getView (&d.view);
        }
    }

  reSize ();
}

void glGrib::Scene::setViewport (int _width, int _height)
{
  d.view.setViewport (_width, _height);
  float width = d.view.getWidth (), height = d.view.getHeight ();
  float ratio = width / height;

  d.MVP_L = glm::ortho (0.0f, ratio, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  
  d.MVP_R = glm::ortho (1.0f - ratio, 1.0f, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
}

void glGrib::Scene::reSize ()
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

glGrib::Options glGrib::Scene::getOptions () const
{
  glGrib::Options o = d.opts;

  o.view           = d.view.getOptions ();
  o.landscape      = d.landscape.getOptions ();
  o.grid           = d.grid.getOptions ();
  o.ticks          = d.ticks.getOptions ();
  o.coast          = d.coast.getOptions ();
  o.border         = d.border.getOptions ();
  o.rivers         = d.rivers.getOptions ();
  o.departements   = d.departements.getOptions ();
  o.cities         = d.cities.getOptions ();
  o.geopoints      = d.geopoints.getOptions ();
  o.mapscale       = d.mapscale.getOptions ();

  for (size_t i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != nullptr)
      o.field[i] = fieldlist[i]->getOptions ();

  return o;
}

void glGrib::Scene::setViewOptions (const glGrib::OptionsView & o)
{
  d.view.setup (o);
}

void glGrib::Scene::setLandscapeOptions (const glGrib::OptionsLandscape & o)
{
  clear (d.landscape);
  if (o.on)
    d.landscape.setup (&ld, o);
}

void glGrib::Scene::setLandOptions (const glGrib::OptionsLand & o)
{
  clear (d.land);
  if (o.on)
    d.land.setup (o);
}

void glGrib::Scene::setGridOptions (const glGrib::OptionsGrid & o)
{
  clear (d.grid);
  if (o.on)
    d.grid.setup (o);
}

void glGrib::Scene::setTicksOptions (const glGrib::OptionsTicks & o)
{
  clear (d.ticks);
  d.ticks.setup (o);
  d.ticks.reSize (d.view);
}

void glGrib::Scene::setCoastOptions (const glGrib::OptionsCoast & o)
{
  clear (d.coast);
  if (o.on)
    d.coast.setup (o);
}

void glGrib::Scene::setBorderOptions (const glGrib::OptionsBorder & o)
{
  clear (d.border);
  if (o.on)
    d.border.setup (o);
}

void glGrib::Scene::setRiversOptions (const glGrib::OptionsRivers & o)
{
  clear (d.rivers);
  if (o.on)
    d.rivers.setup (o);
}

void glGrib::Scene::setDepartementsOptions (const glGrib::OptionsDepartements & o)
{
  clear (d.departements);
  if (o.on)
    d.departements.setup (o);
}

void glGrib::Scene::setFieldOptions (int j, const glGrib::OptionsField & o, float slot)
{
  if (fieldlist[j] != nullptr)
    delete fieldlist[j];

  fieldlist[j] = glGrib::Field::create (o, slot, &ld);
}

void glGrib::Scene::setColorBarOptions (const glGrib::OptionsColorbar & o)
{
  d.opts.colorbar = o;
  clear (d.strmess);
  clear (d.colorbar);

  if (d.opts.colorbar.on)
    {
      glGrib::FontPtr font = getGlGribFontPtr (d.opts.colorbar.font);
      d.strmess.setup (font, std::string (30, ' '), 1.0f, 1.0f, 
                       d.opts.colorbar.font.scale, glGrib::String::NE);
      d.strmess.setForegroundColor (d.opts.colorbar.font.color.foreground);
      d.strmess.setSide (Object2D::RIGHT);
      d.colorbar.setup (d.opts.colorbar);
      updateColorbar ();
    }
}

void glGrib::Scene::setMapScaleOptions (const glGrib::OptionsMapscale & o)
{
  d.opts.mapscale = o;
  clear (d.mapscale);

  if (d.opts.mapscale.on)
    {
      d.mapscale.setup (d.opts.mapscale);
      d.mapscale.reSize (d.view);
    }

}

void glGrib::Scene::setImageOptions (const glGrib::OptionsImage & o)
{
  d.opts.scene.image = o;
  clear (d.image);
  if (d.opts.scene.image.on)
    d.image.setup (d.opts.scene.image);
}

void glGrib::Scene::setTextOptions (const glGrib::OptionsText & o)
{
  d.opts.scene.text = o;
  clear (d.str);
  if (d.opts.scene.text.on)
    {
      glGrib::FontPtr font = getGlGribFontPtr (d.opts.scene.text.font);
      for (size_t i = 0; i < d.opts.scene.text.s.size (); i++)
        {
          glGrib::String2D str;
          d.str.push_back (str);

          if (i >= d.opts.scene.text.x.size ())
            break;
          if (i >= d.opts.scene.text.y.size ())
            break;

          glGrib::String::align_t a = i < d.opts.scene.text.a.size () ? 
            glGrib::String::str2align (d.opts.scene.text.a[i]) : glGrib::String::C;

          d.str[i].setup (font, d.opts.scene.text.s[i], d.opts.scene.text.x[i], 
                          d.opts.scene.text.y[i], d.opts.scene.text.font.scale, a);
          d.str[i].setForegroundColor (d.opts.scene.text.font.color.foreground);
          d.str[i].setBackgroundColor (d.opts.scene.text.font.color.background);
          d.str[i].setSide (Object2D::RIGHT);
        }
    }
}

void glGrib::Scene::setCitiesOptions (const glGrib::OptionsCities & o)
{
  d.opts.cities = o;
  clear (d.cities);
  if (d.opts.cities.on)
    d.cities.setup (o);
}

void glGrib::Scene::setGeoPointsOptions (const glGrib::OptionsGeoPoints & o)
{
  d.opts.geopoints = o;
  clear (d.geopoints);
  if (d.opts.geopoints.on)
    d.geopoints.setup (o);
}

void glGrib::Scene::setGridColorOptions (const glGrib::OptionColor & color)
{
  d.grid.setColorOptions (color);
}

void glGrib::Scene::setGridScaleOptions (float scale)
{
  d.grid.setScaleOptions (scale);
}

void glGrib::Scene::setFieldPaletteOptions (int j, const glGrib::OptionsPalette & opts)
{
  glGrib::Field * fld = fieldlist[j];
  if (fld == nullptr)
    return;
  fld->setPaletteOptions (opts);
}

void glGrib::Scene::setDateOptions (const glGrib::OptionsDate & o)
{
  strdate = "";
  d.opts.scene.date = o;
  clear (d.strdate);
  if (d.opts.scene.date.on)
    {
      glGrib::FontPtr font = getGlGribFontPtr (d.opts.scene.date.font);
      d.strdate.setup (font, std::string (20, ' '), 1.0f, 0.0f, 
                       d.opts.scene.date.font.scale, glGrib::String::SE);
      d.strdate.setSide (Object2D::RIGHT);
      d.strdate.setForegroundColor (d.opts.scene.date.font.color.foreground);
      d.strdate.setBackgroundColor (d.opts.scene.date.font.color.background);
    }
}

void glGrib::Scene::setTitleOptions (const glGrib::OptionsTitle & o)
{
  strtitle = "";
  d.opts.scene.title = o;
  clear (d.strtitle);
  if (d.opts.scene.title.on)
    {
    }
}

void glGrib::Scene::setLightOptions (const glGrib::OptionsLight & o)
{
  d.opts.scene.light = o;
}

void glGrib::Scene::setSceneOptions (const glGrib::OptionsScene & o)
{
  d.opts.scene.rotate_earth = o.rotate_earth;
  d.opts.scene.lon_at_hour  = o.lon_at_hour;
}
