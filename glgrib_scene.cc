#include "glgrib_scene.h"
#include "glgrib_opengl.h"

#include <stdio.h>
#include <sys/time.h>


glgrib_scene & glgrib_scene::operator= (const glgrib_scene & other)
{

  if (this != &other)
    {
      d = other.d;
     
      for (int i = 0; i < fieldlist.size (); i++)
        if (fieldlist[i] != NULL)
          delete fieldlist[i];
     
      fieldlist.clear ();
     
      for (int i = 0; i < other.fieldlist.size (); i++)
        if (other.fieldlist[i] == NULL)
          fieldlist.push_back (NULL);
        else
          fieldlist.push_back (other.fieldlist[i]->clone ());
   }
}

glgrib_scene::~glgrib_scene () 
{
  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != NULL)
      delete fieldlist[i];
}

void glgrib_scene::display_obj (const glgrib_object * obj) const
{
  if (obj == NULL)
    return;
  if (! obj->isReady ())
    return;
  if (! obj->visible ())
    return;
  obj->render (d.view, d.opts.scene.light);
}

void glgrib_scene::display () const
{
  d.view.calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//d.test.render (d.view, d.opts.scene.light);
  
  display_obj (&d.landscape);
  display_obj (&d.coast);
  display_obj (&d.border);
  display_obj (&d.rivers);
  display_obj (&d.departements);
  display_obj (&d.grid);

  for (int i = 0; i < fieldlist.size (); i++)
    display_obj (fieldlist[i]);

  const glgrib_field * fld = d.currentFieldRank < fieldlist.size () 
                           ? fieldlist[d.currentFieldRank] : NULL;
  d.image.render (d.MVP_L);

  if ((fld != NULL) && (! d.colorbar.getHidden ()))
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

  if (d.opts.scene.test_strxyz.on)
    d.strxyz.render (d.view);

  for (std::vector<glgrib_string>::const_iterator it = d.str.begin ();
       it != d.str.end (); it++)
    it->render (d.MVP_R);

  if (d.opts.cities.on)
    d.cities.render (d.view);

}

const glgrib_option_date * glgrib_scene::get_date ()
{
  for (int i = 0; i < fieldlist.size (); i++)
    {
    if (fieldlist[i])
      if (fieldlist[i]->isReady ())
        {
          glgrib_field * fld = fieldlist[i];
          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
	  if (meta.size () >= 1)
            return &meta[0].term;
        }
    }
  return NULL;
}


void glgrib_scene::update_light ()
{
  if (d.opts.scene.light.rotate.on)
    d.opts.scene.light.lon -= 1.;

  const glgrib_option_date * date = NULL;

  if (d.opts.scene.light.date_from_grib.on)
    date = get_date ();
  else if (d.opts.scene.light.date.year != 0)
    date = &d.opts.scene.light.date;

  if (date != NULL)
    {
      const int nday[12] = {31, 28, 31, 30, 31, 30, 
                            31, 31, 30, 31, 30, 31};
      const float dtrop = 23.0f + 26.0f / 60.0f;  // Tropics
      const float eday  = 31 + 28 + 20;           // 20th of March, equinox
      float cday = date->day-1;
      float time = (date->hour * 60.0f + date->minute) * 60.0f + date->second;
      for (int m = 0; m < date->month-1; m++)
        cday += nday[m];
      d.opts.scene.light.lat = dtrop * sin (2.0f * M_PI * (cday - eday) / 365.0f);
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

void glgrib_scene::update_view ()
{
  glgrib_options_view o = d.view.getOptions ();
  d.view.calcMVP ();  

  if (d.opts.scene.rotate_earth.on)
    o.lon += 1.;
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
      const glgrib_option_date * date = get_date ();
      if (date != NULL)
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
}

void glgrib_scene::update_interpolation ()
{
  if (d.opts.scene.interpolation.on)
    {
      int slotmax = 0;

      for (int i = 0; i < fieldlist.size (); i++)
        {
	  if (fieldlist[i] != NULL)
            {
              if (d.opts.field[i].vector.on)
                slotmax = std::max (slotmax, (int)d.opts.field[i].path.size () / 2);
              else if (d.opts.field[i].stream.on)
                slotmax = std::max (slotmax, (int)d.opts.field[i].path.size () / 2);
              else if (d.opts.field[i].contour.on)
                slotmax = std::max (slotmax, (int)d.opts.field[i].path.size ());
              else
                slotmax = std::max (slotmax, (int)d.opts.field[i].path.size ());
	    }
	}
      slotmax--;

      float slot = (float)d.nupdate / (float)d.opts.scene.interpolation.frames;
      if (slot > slotmax)
        slot = slotmax;


      for (int j = 0; j < fieldlist.size (); j++)
        setFieldOptions (j, d.opts.field[j], slot);

    }
}


void glgrib_scene::update_date ()
{
  if (d.opts.scene.date.on)
    {
      const glgrib_option_date * date = NULL;

      glgrib_field * fld = getCurrentField ();
      if (fld != NULL)
        {
          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
          date = &meta[0].term;
	}
      else
        {
          date = get_date ();
	}

      if (date != NULL)
        {
          if (strdate != date->asString ())
            {
              strdate = date->asString ();
              d.strdate.update (strdate);
            }
	}
    }
}

void glgrib_scene::update_title ()
{
  if (d.opts.scene.title.on)
    {
      std::string title = d.opts.scene.title.text;
      glgrib_field * fld = getCurrentField ();
      if ((fld != NULL) && (title == ""))
        {
          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
          title = meta[0].getName ();
	}
      if (strtitle != title)
        {
          d.strtitle.clear ();
          glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.scene.title.font);
          d.strtitle.setup2D (font, title, d.opts.scene.title.x, 
                              d.opts.scene.title.y, d.opts.scene.title.font.scale, 
                              glgrib_string::str2align (d.opts.scene.title.a));
          d.strtitle.setForegroundColor (d.opts.scene.title.font.color.foreground);
          d.strtitle.setBackgroundColor (d.opts.scene.title.font.color.background);
        }
    }
}

void glgrib_scene::update ()
{

  update_view ();
  update_light ();
  update_interpolation ();
  update_date ();
  update_title ();

  d.nupdate++;

}

void glgrib_scene::setup (const glgrib_options & o)
{
  d.opts = o;

//d.test.setup ();

  setViewport (d.opts.window.width, d.opts.window.height);
  setLightOptions (d.opts.scene.light);
  setImageOptions (d.opts.scene.image);

  if (d.opts.scene.interpolation.on)
    {
      int size = 0;
      for (int i = 0; i < d.opts.field.size (); i++)
        {
          bool defined = d.opts.field[i].path.size () != 0;
	  if (! defined)
            continue;
          size += 4;
	}
      ld.setSize (size);
    }

  setLandscapeOptions (d.opts.landscape);
  setGridOptions (d.opts.grid);
  setCoastOptions (d.opts.coast);
  setBorderOptions (d.opts.border);
  setRiversOptions (d.opts.rivers);
  setDepartementsOptions (d.opts.departements);
  setViewOptions (d.opts.view);

  for (int i = 0; i < d.opts.field.size (); i++)
    fieldlist.push_back ((glgrib_field *)NULL);

  for (int i = 0; i < d.opts.field.size (); i++)
    setFieldOptions (i, d.opts.field[i]);

  setDateOptions (d.opts.scene.date);
  setTitleOptions (d.opts.scene.title);
  setTextOptions (d.opts.scene.text);
  setCitiesOptions (d.opts.cities);
  setColorBarOptions (d.opts.colorbar);
  setMapScaleOptions (d.opts.mapscale);

  if (d.opts.scene.test_strxyz.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.scene.date.font);
      d.strxyz.setShared (true);
      d.strxyz.setChange (false);
      d.strxyz.setup3D (font, std::vector<std::string>{"ABCD","EFGH","IJKL","MNOP"},
                       std::vector<float>{+1.01f,-1.01f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.00f,+0.00f,+1.01f,+0.707*0.000f},
                       std::vector<float>{+0.00f,+0.00f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.0f,+0.0f,+90.0f,+0.0f},
                       d.opts.scene.date.font.scale, glgrib_string::C);
      d.strxyz.setForegroundColor (d.opts.scene.date.font.color.foreground);
    }

  d.currentFieldRank = d.opts.scene.select.field;

  resize ();
}

void glgrib_scene::setViewport (int _width, int _height)
{
  d.view.setViewport (_width, _height);
  float width = d.view.getWidth (), height = d.view.getHeight ();
  float ratio = width / height;

  d.MVP_L = glm::ortho (0.0f, ratio, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  
  d.MVP_R = glm::ortho (1.0f - ratio, 1.0f, 0.0f, 1.0f, 0.1f, 100.0f)
          * glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
}

void glgrib_scene::resize ()
{
  d.landscape.resize (d.view);
  d.coast.resize (d.view);
  d.border.resize (d.view);
  d.rivers.resize (d.view);
  d.departements.resize (d.view);
  d.grid.resize (d.view);
  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i])
      fieldlist[i]->resize (d.view);
}

glgrib_options glgrib_scene::getOptions () const
{
  glgrib_options o = d.opts;

  o.view           = d.view.getOptions ();
  o.landscape      = d.landscape.getOptions ();
  o.grid           = d.grid.getOptions ();
  o.coast          = d.coast.getOptions ();
  o.border         = d.border.getOptions ();
  o.rivers         = d.rivers.getOptions ();
  o.departements   = d.departements.getOptions ();
  o.cities         = d.cities.getOptions ();
  o.mapscale       = d.mapscale.getOptions ();

  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != NULL)
      o.field[i] = fieldlist[i]->getOptions ();

  return o;
}

void glgrib_scene::setViewOptions (const glgrib_options_view & o)
{
  d.view.setup (o);
}

void glgrib_scene::setLandscapeOptions (const glgrib_options_landscape & o)
{
  d.landscape.clear ();
  if (o.on)
    d.landscape.setup (&ld, o);
}

void glgrib_scene::setGridOptions (const glgrib_options_grid & o)
{
  d.grid.clear ();
  if (o.on)
    d.grid.setup (o);
}

void glgrib_scene::setCoastOptions (const glgrib_options_coast & o)
{
  d.coast.clear ();
  if (o.on)
    d.coast.setup (o);
}

void glgrib_scene::setBorderOptions (const glgrib_options_border & o)
{
  d.border.clear ();
  if (o.on)
    d.border.setup (o);
}

void glgrib_scene::setRiversOptions (const glgrib_options_rivers & o)
{
  d.rivers.clear ();
  if (o.on)
    d.rivers.setup (o);
}

void glgrib_scene::setDepartementsOptions (const glgrib_options_departements & o)
{
  d.departements.clear ();
  if (o.on)
    d.departements.setup (o);
}

void glgrib_scene::setFieldOptions (int j, const glgrib_options_field & o, float slot)
{
  if (fieldlist[j] != NULL)
    delete fieldlist[j];

  fieldlist[j] = glgrib_field::create (o, slot, &ld);
}

void glgrib_scene::setColorBarOptions (const glgrib_options_colorbar & o)
{
  d.opts.colorbar = o;
  d.strmess.clear ();
  d.colorbar.clear ();

  if (d.opts.colorbar.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.colorbar.font);
      d.strmess.setup2D (font, std::string (30, ' '), 1.0f, 1.0f, 
                        d.opts.colorbar.font.scale, glgrib_string::NE);
      d.strmess.setForegroundColor (d.opts.colorbar.font.color.foreground);
      d.colorbar.setup (d.opts.colorbar);
    }

}

void glgrib_scene::setMapScaleOptions (const glgrib_options_mapscale & o)
{
  d.opts.mapscale = o;
  d.mapscale.clear ();

  if (d.opts.mapscale.on)
    d.mapscale.setup (d.opts.mapscale);

}

void glgrib_scene::setImageOptions (const glgrib_options_image & o)
{
  d.opts.scene.image = o;
  d.image.clear ();
  if (d.opts.scene.image.on)
    d.image.setup (d.opts.scene.image);
}

void glgrib_scene::setTextOptions (const glgrib_options_text & o)
{
  d.opts.scene.text = o;
  d.str.clear ();
  if (d.opts.scene.text.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.scene.text.font);
      for (int i = 0; i < d.opts.scene.text.s.size (); i++)
        {
          glgrib_string str;
          d.str.push_back (str);

          if (i >= d.opts.scene.text.x.size ())
            break;
          if (i >= d.opts.scene.text.y.size ())
            break;

          glgrib_string::align_t a = i < d.opts.scene.text.a.size () ? 
            glgrib_string::str2align (d.opts.scene.text.a[i]) : glgrib_string::C;

          d.str[i].setup2D (font, d.opts.scene.text.s[i], d.opts.scene.text.x[i], 
                           d.opts.scene.text.y[i], d.opts.scene.text.font.scale, a);
          d.str[i].setForegroundColor (d.opts.scene.text.font.color.foreground);
          d.str[i].setBackgroundColor (d.opts.scene.text.font.color.background);
        }
    }
}

void glgrib_scene::setCitiesOptions (const glgrib_options_cities & o)
{
  d.opts.cities = o;
  d.cities.clear ();
  if (d.opts.cities.on)
    d.cities.setup (o);
}

void glgrib_scene::setGridColorOptions (const glgrib_option_color & color)
{
  d.grid.setColorOptions (color);
}

void glgrib_scene::setGridScaleOptions (float scale)
{
  d.grid.setScaleOptions (scale);
}

void glgrib_scene::setFieldPaletteOptions (int j, const glgrib_options_palette & opts)
{
  glgrib_field * fld = fieldlist[j];
  if (fld == NULL)
    return;
  fld->setPaletteOptions (opts);
}

void glgrib_scene::setDateOptions (const glgrib_options_date & o)
{
  strdate = "";
  d.opts.scene.date = o;
  d.strdate.clear ();
  if (d.opts.scene.date.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.scene.date.font);
      d.strdate.setup2D (font, std::string (20, ' '), 1.0f, 0.0f, 
                        d.opts.scene.date.font.scale, glgrib_string::SE);
      d.strdate.setForegroundColor (d.opts.scene.date.font.color.foreground);
      d.strdate.setBackgroundColor (d.opts.scene.date.font.color.background);
    }
}

void glgrib_scene::setTitleOptions (const glgrib_options_title & o)
{
  strtitle = "";
  d.opts.scene.title = o;
  d.strtitle.clear ();
  if (d.opts.scene.title.on)
    {
    }
}

void glgrib_scene::setLightOptions (const glgrib_options_light & o)
{
  d.opts.scene.light = o;
}

void glgrib_scene::setSceneOptions (const glgrib_options_scene & o)
{
  d.opts.scene.rotate_earth = o.rotate_earth;
  d.opts.scene.lon_at_hour  = o.lon_at_hour;
}
