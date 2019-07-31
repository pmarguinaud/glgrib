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
  obj->render (d.view, d.light);
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
  if (fld != NULL)
    d.colorbar.render (d.MVP_L, 
                       fld->dopts.palette, 
                       fld->getNormedMinValue (), 
                       fld->getNormedMaxValue ());

  d.strmess.render (d.MVP_R);

  if (d.opts.scene.display_date)
    d.strdate.render (d.MVP_R);

  if (d.opts.scene.test_strxyz)
    d.strxyz.render (d.view);


}

void glgrib_scene::update_light ()
{
  if (d.light.rotate)
    d.light.lon -= 1.;

  const glgrib_option_date * date = NULL;

  if (d.light.date_from_grib)
    {
      for (int i = 0; i < fieldlist.size (); i++)
        if (fieldlist[i]->isReady ())
          {
            glgrib_field * fld = fieldlist[i];
            const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
            date = &meta[0].term;
            break;
          }
    }
  else if (d.light.date.year != 0)
    {
      date = &d.light.date;
    }

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
      d.light.lat = dtrop * sin (2.0f * M_PI * (cday - eday) / 365.0f);
      d.light.lon = 360.0f * ((12.0f * 3600.0f - time) / (24.0f * 3600.0f));
    }
}

void glgrib_scene::update_view ()
{
  if (d.rotate_earth)
    d.view.opts.lon += 1.;
  if (d.opts.scene.travelling.on)
    {
      float frames = d.opts.scene.travelling.frames;
      float a = (float)d.nupdate / frames;
      if (a <= 1)
        {
          float lon1 = d.opts.scene.travelling.pos1.lon;
          float lon2 = d.opts.scene.travelling.pos2.lon;
          float dlon;
          if (fabs (lon1 - lon2) < 180.0f)
            {
              dlon = lon2 - lon1;
            }
          else
            {
              lon2 += 360.0f;
              dlon = lon2 - lon1;
            }
          
          float lat1 = d.opts.scene.travelling.pos1.lat;
          float lat2 = d.opts.scene.travelling.pos2.lat;
          float dlat = lat2 - lat1;

          float fov1 = d.opts.scene.travelling.pos1.fov;
          float fov2 = d.opts.scene.travelling.pos2.fov;
          float dfov = fov2 - fov1;

          d.view.opts.lon = lon1 + dlon * a;
          d.view.opts.lat = lat1 + dlat * a;
          d.view.opts.fov = fov1 + dfov * a;

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

  if (d.opts.scene.interpolation.on)
    {
      int size = 0;
      for (int i = 0; i < d.opts.field.size (); i++)
        {
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
    d.landscape.init (d.opts.landscape);

  if (d.opts.grid.on)
    d.grid.init (d.opts.grid);

  if (d.opts.coastlines.on)
    d.coastlines.init (d.opts.coastlines);

  d.light = d.opts.scene.light;

  d.view.init (d.opts);

  bool seen_date = false;
  for (int i = 0; i < d.opts.field.size (); i++)
    {
      glgrib_field * fld = NULL;
      bool defined = d.opts.field[i].path.size () != 0;

      if (defined)
        {
          if (d.opts.field[i].vector.on)
            fld = new glgrib_field_vector ();
          else if (d.opts.field[i].contour.on)
            fld = new glgrib_field_contour ();
          else
            fld = new glgrib_field_scalar ();
          fld->init (&ld, d.opts.field[i]);
        }

      fieldlist.push_back (fld);

      if (defined)
        setCurrentFieldRank (i);

      if (defined && (! seen_date) && d.opts.scene.display_date)
        {
          seen_date = true;
    
          glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
          d.strdate.init2D (font, std::string (20, 'X'), 1.0f, 0.0f, d.opts.font.scale, glgrib_string::SE);

          d.strdate.setForegroundColor (d.opts.font.color.foreground.r / 255.0f, 
                                        d.opts.font.color.foreground.g / 255.0f, 
                                        d.opts.font.color.foreground.b / 255.0f,
                                        d.opts.font.color.foreground.a / 255.0f);

          d.strdate.setBackgroundColor (d.opts.font.color.background.r / 255.0f, 
                                        d.opts.font.color.background.g / 255.0f, 
                                        d.opts.font.color.background.b / 255.0f,
                                        d.opts.font.color.background.a / 255.0f);

          const std::vector<glgrib_field_metadata> & meta = fld->getMeta ();
          d.strdate.update (meta[0].term.asString ());
        }

    }

  if (d.opts.colorbar.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      d.strmess.init2D (font, std::string (30, ' '), 1.0f, 1.0f, d.opts.font.scale, glgrib_string::NE);
      d.strmess.setForegroundColor (d.opts.font.color.foreground.r / 255.0f, 
		                    d.opts.font.color.foreground.g / 255.0f, 
		                    d.opts.font.color.foreground.b / 255.0f);
      d.colorbar.init (d.opts.colorbar);
    }
  if (d.opts.scene.test_strxyz)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      d.strxyz.init3D (font, std::vector<std::string>{"ABCD","EFGH","IJKL","MNOP"},
                       std::vector<float>{+1.01f,-1.01f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.00f,+0.00f,+1.01f,+0.707*0.000f},
                       std::vector<float>{+0.00f,+0.00f,+0.00f,+0.707*1.010f},
                       std::vector<float>{+0.0f,+0.0f,+90.0f,+0.0f},
                       d.opts.font.scale, glgrib_string::C);
      d.strxyz.setForegroundColor (d.opts.font.color.foreground.r / 255.0f, 
		                   d.opts.font.color.foreground.g / 255.0f, 
		                   d.opts.font.color.foreground.b / 255.0f);
    }

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


