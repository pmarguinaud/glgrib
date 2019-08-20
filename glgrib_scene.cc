#include "glgrib_scene.h"
#include "glgrib_opengl.h"
#include "glgrib_field_scalar.h"
#include "glgrib_field_vector.h"
#include "glgrib_field_contour.h"

#include <stdio.h>
#include <sys/time.h>


glgrib_scene & glgrib_scene::operator= (const glgrib_scene & other)
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
  
  display_obj (&d.landscape);
  display_obj (&d.coastlines);
  display_obj (&d.grid);

  for (int i = 0; i < fieldlist.size (); i++)
    display_obj (fieldlist[i]);

  const glgrib_field * fld = d.currentFieldRank < fieldlist.size () 
                           ? fieldlist[d.currentFieldRank] : NULL;
  d.image.render (d.MVP_L);

  if ((fld != NULL) && (! d.colorbar.getHidden ()))
    if (fld->getKind () == glgrib_field::SCALAR)
      d.colorbar.render (d.MVP_L, 
                         fld->palette, 
                         fld->getNormedMinValue (), 
                         fld->getNormedMaxValue ());

  d.strmess.render (d.MVP_R);

  if (d.opts.scene.display_date)
    d.strdate.render (d.MVP_R);

  if (d.opts.scene.test_strxyz)
    d.strxyz.render (d.view);

  for (std::vector<glgrib_string>::const_iterator it = d.str.begin ();
       it != d.str.end (); it++)
    it->render (d.MVP_R);


}

const glgrib_option_date * glgrib_scene::get_date ()
{
  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i])
      if (fieldlist[i]->isReady ())
        {
          glgrib_field * fld = fieldlist[i];
          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
          return &meta[0].term;
        }
  return NULL;
}


void glgrib_scene::update_light ()
{
  if (d.opts.scene.light.rotate)
    d.opts.scene.light.lon -= 1.;

  const glgrib_option_date * date = NULL;

  if (d.opts.scene.light.date_from_grib)
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
  if (d.opts.scene.rotate_earth)
    d.view.opts.lon += 1.;
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

          d.view.opts.lon = lon1 + dlon * a;
          d.view.opts.lat = lat1 + dlat * a;
          d.view.opts.fov = fov1 + dfov * a;

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
          d.view.opts.lon = lon;
        }
    }
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

      bool seen_date = false;
      for (int i = 0; i < fieldlist.size (); i++)
        {
	  if (fieldlist[i] != NULL)
            {
              delete fieldlist[i];
	      fieldlist[i] = NULL;

              glgrib_field * fld = NULL;

              if (d.opts.field[i].vector.on)
                fld = new glgrib_field_vector ();
              else if (d.opts.field[i].contour.on)
                fld = new glgrib_field_contour ();
              else
                fld = new glgrib_field_scalar ();

              fld->init (&ld, d.opts.field[i], slot);
	      fieldlist[i] = fld;

              if ((! seen_date) && (d.opts.scene.display_date))
                {
                  const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
                  d.strdate.update (meta[0].term.asString ());
                  seen_date = true;
                }
	    }
	}
    }
}

void glgrib_scene::update ()
{

  update_view ();
  update_light ();
  update_interpolation ();

  d.nupdate++;

}

void glgrib_scene::init (const glgrib_options & o)
{
  d.opts = o;

  setViewport (d.opts.window.width, d.opts.window.height);

  if (d.opts.scene.light.on)
    setLight ();

  if (d.opts.scene.image.on) 
    d.image.init (d.opts.scene.image);

  if (d.opts.scene.interpolation.on)
    {
      int size = 0;
      for (int i = 0; i < d.opts.field.size (); i++)
        {
          bool defined = d.opts.field[i].path.size () != 0;
	  if (! defined)
            continue;
          if (d.opts.field[i].vector.on)
            size += 4;
          else if (d.opts.field[i].contour.on)
            size += 2;
          else
            size += 2;
	}
      ld.setSize (size);
    }

  if (d.opts.landscape.on)
    d.landscape.init (&ld, d.opts.landscape);

  if (d.opts.grid.on)
    d.grid.init (d.opts.grid);

  if (d.opts.coastlines.on)
    d.coastlines.init (d.opts.coastlines);

  d.view.init (d.opts.view);

  for (int i = 0; i < d.opts.field.size (); i++)
    fieldlist.push_back ((glgrib_field *)NULL);

  for (int i = 0; i < d.opts.field.size (); i++)
    setFieldOpts (i, d.opts.field[i]);

  bool seen_date = false;
  for (int i = 0; i < d.opts.field.size (); i++)
    {
      glgrib_field * fld = fieldlist[i];
      if ((fld != NULL) && (! seen_date) && d.opts.scene.display_date)
        {
          seen_date = true;
    
          glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
          d.strdate.init2D (font, std::string (20, 'X'), 1.0f, 0.0f, 
                            d.opts.font.scale, glgrib_string::SE);

          d.strdate.setForegroundColor (d.opts.font.color.foreground);
          d.strdate.setBackgroundColor (d.opts.font.color.background);

          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
          d.strdate.update (meta[0].term.asString ());
        }
    }


  for (int i = 0; i < d.opts.scene.text.size (); i++)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      glgrib_string str;
      d.str.push_back (str);

      if (i >= d.opts.scene.text_x.size ())
        break;
      if (i >= d.opts.scene.text_y.size ())
        break;

      glgrib_string::align_t a = i < d.opts.scene.text_a.size () ? 
        glgrib_string::str2align (d.opts.scene.text_a[i]) : glgrib_string::C;

      d.str[i].init2D (font, d.opts.scene.text[i], d.opts.scene.text_x[i], d.opts.scene.text_y[i], d.opts.font.scale, a);
      d.str[i].setForegroundColor (d.opts.font.color.foreground);
      d.str[i].setBackgroundColor (d.opts.font.color.background);

    }


  setColorBarOpts (d.opts.colorbar);

  if (d.opts.scene.test_strxyz)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      d.strxyz.init3D (font, std::vector<std::string>{"ABCD","EFGH","IJKL","MNOP"},
                       std::vector<float>{+1.01f,-1.01f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.00f,+0.00f,+1.01f,+0.707*0.000f},
                       std::vector<float>{+0.00f,+0.00f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.0f,+0.0f,+90.0f,+0.0f},
                       d.opts.font.scale, glgrib_string::C);
      d.strxyz.setForegroundColor (d.opts.font.color.foreground);
    }

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
  d.coastlines.resize (d.view);
  d.grid.resize (d.view);
  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i])
      fieldlist[i]->resize (d.view);
}

glgrib_options glgrib_scene::getOptions () const
{
  glgrib_options o = d.opts;
  o.view       = d.view.opts;
  o.landscape  = d.landscape.opts;
  o.grid       = d.grid.opts;
  o.coastlines = d.coastlines.opts;

  for (int i = 0; i < fieldlist.size (); i++)
    if (fieldlist[i] != NULL)
      o.field[i] = fieldlist[i]->opts;

  return o;
}

void glgrib_scene::setViewOpts (const glgrib_options_view & o)
{
  d.view.opts = o;
}

void glgrib_scene::setLandscapeOpts (const glgrib_options_landscape & o)
{
  glgrib_landscape landscape;
  if (o.on)
    landscape.init (&ld, o);
  d.landscape = landscape;
}

void glgrib_scene::setGridOpts (const glgrib_options_grid & o)
{
  glgrib_grid grid;
  if (o.on)
    grid.init (o);
  d.grid = grid;
}

void glgrib_scene::setCoastlinesOpts (const glgrib_options_coastlines & o)
{
  glgrib_coastlines coastlines;
  if (o.on)
    coastlines.init (o);
  d.coastlines = coastlines;
}

void glgrib_scene::setFieldOpts (int j, const glgrib_options_field & o)
{
  if (fieldlist[j] != NULL)
    delete fieldlist[j];
  fieldlist[j] = NULL; 

  bool defined = o.path.size () != 0;

  if (defined)
    {
      glgrib_field * fld = NULL;
      if (o.vector.on)
        fld = new glgrib_field_vector ();
      else if (o.contour.on)
        fld = new glgrib_field_contour ();
      else
        fld = new glgrib_field_scalar ();
      fld->init (&ld, o);
      fieldlist[j] = fld;
    }

}

void glgrib_scene::setColorBarOpts (const glgrib_options_colorbar & o)
{
  d.opts.colorbar = o;
  d.strmess.cleanup ();
  d.colorbar.cleanup ();

  if (d.opts.colorbar.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      d.strmess.init2D (font, std::string (30, ' '), 1.0f, 1.0f, 
                        d.opts.font.scale, glgrib_string::NE);
      d.strmess.setForegroundColor (d.opts.font.color.foreground);
      d.colorbar.init (d.opts.colorbar);
    }

}




