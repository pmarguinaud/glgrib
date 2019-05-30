#include "glgrib_scene.h"
#include "glgrib_opengl.h"
#include "glgrib_field_scalar.h"

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

void glgrib_scene::setLightShader (GLuint programID) const
{
  int lightid = glGetUniformLocation (programID, "light");
  
  if (lightid != -1)
    {
      glUniform1i (lightid, d.light.on);
      if (d.light.on)
        {
          const double deg2rad = M_PI / 180.0;
          float coslon = cos (deg2rad * d.light.lon);
          float sinlon = sin (deg2rad * d.light.lon);
          float coslat = cos (deg2rad * d.light.lat);
          float sinlat = sin (deg2rad * d.light.lat);
	  glm::vec3 lightDir = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
          glUniform3fv (glGetUniformLocation (programID, "lightDir"), 
                        1, &lightDir[0]);
        }
    }
}

void glgrib_scene::display_obj (const glgrib_object * obj) const
{
  if (! obj->isReady ())
    return;

  const glgrib_program * program = obj->get_program ();
  program->use ();
  d.view.setMVP (program->programID);
  setLightShader (program->programID);
  obj->render (&d.view);
}

void glgrib_scene::display () const
{
  d.view.calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (d.landscape.visible ())
    display_obj (&d.landscape);

  if (d.coastlines.visible ())
    display_obj (&d.coastlines);

  if (d.grid.visible ())
    display_obj (&d.grid);

  for (int i = 0; i < fieldlist.size (); i++)
    {
      const glgrib_field * fld = fieldlist[i];
      if (fld == NULL)
        continue;
      if (! fld->isReady ())
        continue;
      if (fld->visible ())
        {
          const glgrib_program * program = fld->get_program ();
          program->use ();
          d.view.setMVP (program->programID);
          setLightShader (program->programID);
          fld->render (&d.view);
        }
    }

  const glgrib_field * fld = d.currentFieldRank < fieldlist.size () 
                           ? fieldlist[d.currentFieldRank] : NULL;
  if (fld != NULL)
    d.colorbar.render (d.MVP_L, 
                       fld->dopts.palette, 
                       fld->getNormedMinValue (), 
                       fld->getNormedMaxValue ());

  d.str.render (d.MVP_R);


}

static double current_time ()
{
  struct timeval tv; 
  struct timezone tz;
  gettimeofday (&tv, &tz);
  return tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

void glgrib_scene::update ()
{
  double t = current_time ();
  if (d.rotate_earth)
    d.view.opts.lon += 1.;
  if (d.light.rotate)
    d.light.lon -= 1.;


  if (d.movie && ((t - d.movie_time) > d.opts.scene.movie_wait))
    {
      bool advance = false;

      for (int i = 0; i < fieldlist.size (); i++)
        {
          glgrib_field * fld = NULL;
          bool defined = d.opts.field[i].path.size () != 0;
	  if (defined)
            {
	      int size = d.opts.field[i].path.size ();
	      advance = d.movie_index < size;
	      int slot = advance ? d.movie_index : size-1;
              fld = new glgrib_field_scalar ();
              fld->init (d.opts.field[i], slot);
	      fieldlist[i] = fld;
	    }
	}

      if (advance)
        d.movie_index++;
      else
        d.movie_index = 0;

      d.movie_time = t;
    }
}

void glgrib_scene::init (const glgrib_options & o)
{
  d.opts = o;

  if (d.opts.landscape.path != "")
    d.landscape.init (d.opts.landscape);

  if (d.opts.grid.resolution)
    d.grid.init (d.opts.grid);

  if (d.opts.coastlines.path != "")
    d.coastlines.init (d.opts.coastlines);

  d.light = d.opts.scene.light;

  d.view.init (d.opts);

  for (int i = 0; i < d.opts.field.size (); i++)
    {
      glgrib_field * fld = NULL;
      bool defined = d.opts.field[i].path.size () != 0;

      if (defined)
        {
          fld = new glgrib_field_scalar ();
          fld->init (d.opts.field[i]);
        }

      fieldlist.push_back (fld);

      if (defined)
        setCurrentFieldRank (i);

    }

  if (d.opts.colorbar.on)
    {
      glgrib_font_ptr font = new_glgrib_font_ptr (d.opts.font);
      d.str.init (font, std::string (30, ' '), 1.0f, 1.0f, d.opts.font.scale, glgrib_string::NE);
      d.str.setColor (d.opts.font.r / 255.0f, d.opts.font.g / 255.0f, d.opts.font.b / 255.0f);
      d.colorbar.init (d.opts.colorbar);
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


