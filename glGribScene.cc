#include "glGribScene.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"

#include <stdio.h>
#include <sys/time.h>


glGribScene & glGribScene::operator= (const glGribScene & other)
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

glGribScene::~glGribScene () 
{
  for (auto f : fieldlist)
    if (f != nullptr)
      delete f;
}

void glGribScene::displayObj (const glGribObject * obj) const
{
  if (obj == nullptr)
    return;
  if (! obj->isReady ())
    return;
  if (! obj->visible ())
    return;
  obj->render (d.view, d.opts.scene.light);
}

void glGribScene::display () const
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  d.test.render (d.view, d.opts.scene.light);
  

  std::vector<const glGribObject*> obj_list;

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
  obj_list.push_back (&d.land);


  std::sort (obj_list.begin (), obj_list.end (), 
             [] (const glGribObject * a, const glGribObject * b) 
             { return a->getScale () < b->getScale (); });


  for (auto obj : obj_list)
    displayObj (obj);

  const glGribField * fld = d.currentFieldRank < fieldlist.size () 
                           ? fieldlist[d.currentFieldRank] : nullptr;
  d.image.render (d.MVP_L);

  if ((fld != nullptr) && (! d.colorbar.getHidden ()))
    if (fld->useColorBar ())
      d.colorbar.render (d.MVP_L, 
                         fld->getPalette (), 
                         fld->getNormedMinValue (), 
                         fld->getNormedMaxValue ());

  d.mapscale.render (d.MVP_L, d.view);

  d.strmess.render (d.MVP_R);

  if (d.opts.scene.date.on)
    d.strdate.render (d.MVP_R);

  if (d.opts.scene.title.on)
    d.strtitle.render (d.MVP_L);

  d.ticks.render (d.MVP_L);

  for (auto str : d.str)
    str.render (d.MVP_R);

}

const glGribOptionDate * glGribScene::getDate ()
{
  for (auto fld : fieldlist)
    if (fld)
      if (fld->isReady ())
        {
          const std::vector<glGribFieldMetadata> & meta = fld->getMeta ();
	  if (meta.size () >= 1)
            return &meta[0].term;
        }
  return nullptr;
}


void glGribScene::updateLight ()
{
  if (d.opts.scene.light.rotate.on)
    d.opts.scene.light.lon -= d.opts.scene.light.rotate.rate;

  const glGribOptionDate * date = nullptr;

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

static float ffmod (float x, float y)
{
  while (x < 0)
    x += y;
  while (x > y)
    x -= y;
  return x;
}

void glGribScene::updateView ()
{
  glGribOptionsView o = d.view.getOptions ();

  if (d.opts.scene.rotate_earth.on)
    o.lon += d.opts.scene.rotate_earth.rate;
  if (d.opts.scene.travelling.on)
    {
      float frames = d.opts.scene.travelling.frames;
      float a = (float)d.nupdate / frames;
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

	  resize ();
        }
    }
  else if ((d.opts.scene.lon_at_hour >= 0.0f) && 
           (d.opts.scene.lon_at_hour <= 24.0f))
    {
      const glGribOptionDate * date = getDate ();
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

  d.ticks.resize (d.view);
}

void glGribScene::updateInterpolation ()
{
  if (d.opts.scene.interpolation.on)
    {
      int slotmax = 0;

      for (auto f : fieldlist)
        if (f != nullptr)
          slotmax = std::max (slotmax, f->getSlotMax ());

      slotmax--;

      float slot = (float)d.nupdate / (float)d.opts.scene.interpolation.frames;
      if (slot > slotmax)
        slot = slotmax;

      for (int j = 0; j < fieldlist.size (); j++)
        setFieldOptions (j, d.opts.field[j], slot);
    }
}


void glGribScene::updateDate ()
{
  if (d.opts.scene.date.on)
    {
      const glGribOptionDate * date = nullptr;

      glGribField * fld = getCurrentField ();
      if (fld != nullptr)
        {
          const std::vector<glGribFieldMetadata> & meta = fld->getMeta ();
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

void glGribScene::updateTitle ()
{
  if (d.opts.scene.title.on)
    {
      std::string title = d.opts.scene.title.text;
      glGribField * fld = getCurrentField ();
      if ((fld != nullptr) && (title == ""))
        {
          const std::vector<glGribFieldMetadata> & meta = fld->getMeta ();
          title = meta[0].getName ();
	}
      if (strtitle != title)
        {
          d.strtitle.clear ();
          glgrib_font_ptr font = newGlgribFontPtr (d.opts.scene.title.font);
          d.strtitle.setup2D (font, title, d.opts.scene.title.x, 
                              d.opts.scene.title.y, d.opts.scene.title.font.scale, 
                              glGribString::str2align (d.opts.scene.title.a));
          d.strtitle.setForegroundColor (d.opts.scene.title.font.color.foreground);
          d.strtitle.setBackgroundColor (d.opts.scene.title.font.color.background);
        }
    }
}

void glGribScene::update ()
{

  updateView ();
  updateLight ();
  updateInterpolation ();
  updateDate ();
  updateTitle ();

  d.nupdate++;

}

void glGribScene::setup (const glGribOptions & o)
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
    fieldlist.push_back ((glGribField *)nullptr);

  for (int i = 0; i < d.opts.field.size (); i++)
    setFieldOptions (i, d.opts.field[i]);

  setDateOptions (d.opts.scene.date);
  setTitleOptions (d.opts.scene.title);
  setTextOptions (d.opts.scene.text);
  setCitiesOptions (d.opts.cities);
  setColorBarOptions (d.opts.colorbar);
  setMapScaleOptions (d.opts.mapscale);

  d.currentFieldRank = d.opts.scene.select.field;


  if (d.opts.scene.center.on)
    {
      glGribField * field = getCurrentField ();
      if (field != nullptr)
        {
          const_glgrib_geometry_ptr geometry = field->getGeometry ();
          geometry->getView (&d.view);
        }
    }

  resize ();
}

void glGribScene::setViewport (int _width, int _height)
{
  d.view.setViewport (_width, _height);
  float width = d.view.getWidth (), height = d.view.getHeight ();
  float ratio = width / height;

  d.MVP_L = glm::ortho (0.0f, ratio, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  
  d.MVP_R = glm::ortho (1.0f - ratio, 1.0f, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
}

void glGribScene::resize ()
{
  d.landscape.resize (d.view);
  d.coast.resize (d.view);
  d.border.resize (d.view);
  d.rivers.resize (d.view);
  d.departements.resize (d.view);
  d.grid.resize (d.view);
  d.ticks.resize (d.view);
  for (auto f : fieldlist)
    if (f)
      f->resize (d.view);
}

glGribOptions glGribScene::getOptions () const
{
  glGribOptions o = d.opts;

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

  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != nullptr)
      o.field[i] = fieldlist[i]->getOptions ();

  return o;
}

void glGribScene::setViewOptions (const glGribOptionsView & o)
{
  d.view.setup (o);
}

void glGribScene::setLandscapeOptions (const glGribOptionsLandscape & o)
{
  d.landscape.clear ();
  if (o.on)
    d.landscape.setup (&ld, o);
}

void glGribScene::setLandOptions (const glGribOptionsLand & o)
{
  d.land.clear ();
  if (o.on)
    d.land.setup (o);
}

void glGribScene::setGridOptions (const glGribOptionsGrid & o)
{
  d.grid.clear ();
  if (o.on)
    d.grid.setup (o);
}

void glGribScene::setTicksOptions (const glGribOptionsTicks & o)
{
  d.ticks.clear ();
  d.ticks.setup (o);
  d.ticks.resize (d.view);
}

void glGribScene::setCoastOptions (const glGribOptionsCoast & o)
{
  d.coast.clear ();
  if (o.on)
    d.coast.setup (o);
}

void glGribScene::setBorderOptions (const glGribOptionsBorder & o)
{
  d.border.clear ();
  if (o.on)
    d.border.setup (o);
}

void glGribScene::setRiversOptions (const glGribOptionsRivers & o)
{
  d.rivers.clear ();
  if (o.on)
    d.rivers.setup (o);
}

void glGribScene::setDepartementsOptions (const glGribOptionsDepartements & o)
{
  d.departements.clear ();
  if (o.on)
    d.departements.setup (o);
}

void glGribScene::setFieldOptions (int j, const glGribOptionsField & o, float slot)
{
  if (fieldlist[j] != nullptr)
    delete fieldlist[j];

  fieldlist[j] = glGribField::create (o, slot, &ld);
}

void glGribScene::setColorBarOptions (const glGribOptionsColorbar & o)
{
  d.opts.colorbar = o;
  d.strmess.clear ();
  d.colorbar.clear ();

  if (d.opts.colorbar.on)
    {
      glgrib_font_ptr font = newGlgribFontPtr (d.opts.colorbar.font);
      d.strmess.setup2D (font, std::string (30, ' '), 1.0f, 1.0f, 
                        d.opts.colorbar.font.scale, glGribString::NE);
      d.strmess.setForegroundColor (d.opts.colorbar.font.color.foreground);
      d.colorbar.setup (d.opts.colorbar);
    }

}

void glGribScene::setMapScaleOptions (const glGribOptionsMapscale & o)
{
  d.opts.mapscale = o;
  d.mapscale.clear ();

  if (d.opts.mapscale.on)
    d.mapscale.setup (d.opts.mapscale);

}

void glGribScene::setImageOptions (const glGribOptionsImage & o)
{
  d.opts.scene.image = o;
  d.image.clear ();
  if (d.opts.scene.image.on)
    d.image.setup (d.opts.scene.image);
}

void glGribScene::setTextOptions (const glGribOptionsText & o)
{
  d.opts.scene.text = o;
  d.str.clear ();
  if (d.opts.scene.text.on)
    {
      glgrib_font_ptr font = newGlgribFontPtr (d.opts.scene.text.font);
      for (int i = 0; i < d.opts.scene.text.s.size (); i++)
        {
          glGribString str;
          d.str.push_back (str);

          if (i >= d.opts.scene.text.x.size ())
            break;
          if (i >= d.opts.scene.text.y.size ())
            break;

          glGribString::align_t a = i < d.opts.scene.text.a.size () ? 
            glGribString::str2align (d.opts.scene.text.a[i]) : glGribString::C;

          d.str[i].setup2D (font, d.opts.scene.text.s[i], d.opts.scene.text.x[i], 
                           d.opts.scene.text.y[i], d.opts.scene.text.font.scale, a);
          d.str[i].setForegroundColor (d.opts.scene.text.font.color.foreground);
          d.str[i].setBackgroundColor (d.opts.scene.text.font.color.background);
        }
    }
}

void glGribScene::setCitiesOptions (const glGribOptionsCities & o)
{
  d.opts.cities = o;
  d.cities.clear ();
  if (d.opts.cities.on)
    d.cities.setup (o);
}

void glGribScene::setGridColorOptions (const glGribOptionColor & color)
{
  d.grid.setColorOptions (color);
}

void glGribScene::setGridScaleOptions (float scale)
{
  d.grid.setScaleOptions (scale);
}

void glGribScene::setFieldPaletteOptions (int j, const glGribOptionsPalette & opts)
{
  glGribField * fld = fieldlist[j];
  if (fld == nullptr)
    return;
  fld->setPaletteOptions (opts);
}

void glGribScene::setDateOptions (const glGribOptionsDate & o)
{
  strdate = "";
  d.opts.scene.date = o;
  d.strdate.clear ();
  if (d.opts.scene.date.on)
    {
      glgrib_font_ptr font = newGlgribFontPtr (d.opts.scene.date.font);
      d.strdate.setup2D (font, std::string (20, ' '), 1.0f, 0.0f, 
                        d.opts.scene.date.font.scale, glGribString::SE);
      d.strdate.setForegroundColor (d.opts.scene.date.font.color.foreground);
      d.strdate.setBackgroundColor (d.opts.scene.date.font.color.background);
    }
}

void glGribScene::setTitleOptions (const glGribOptionsTitle & o)
{
  strtitle = "";
  d.opts.scene.title = o;
  d.strtitle.clear ();
  if (d.opts.scene.title.on)
    {
    }
}

void glGribScene::setLightOptions (const glGribOptionsLight & o)
{
  d.opts.scene.light = o;
}

void glGribScene::setSceneOptions (const glGribOptionsScene & o)
{
  d.opts.scene.rotate_earth = o.rotate_earth;
  d.opts.scene.lon_at_hour  = o.lon_at_hour;
}
