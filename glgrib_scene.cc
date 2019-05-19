#include "glgrib_scene.h"
#include "glgrib_opengl.h"

#include <stdio.h>
#include <sys/time.h>


using namespace glm;

void glgrib_scene::setLightShader (GLuint programID) const
{
  int lightid = glGetUniformLocation (programID, "light");
  
  if (lightid != -1)
    {
      glUniform1i (lightid, light.on);
      if (light.on)
        {
          const double deg2rad = M_PI / 180.0;
          float coslon = cos (deg2rad * light.lon);
          float sinlon = sin (deg2rad * light.lon);
          float coslat = cos (deg2rad * light.lat);
          float sinlat = sin (deg2rad * light.lat);
          float lightDir[3] = {coslon * coslat, sinlon * coslat, sinlat};
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
  view.setMVP (program->matrixID);
  setLightShader (program->programID);
  obj->render (&view);
}

void glgrib_scene::display () const
{
  view.calcMVP ();  

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (landscape.visible ())
    display_obj (&landscape);

  if (coastlines.visible ())
    display_obj (&coastlines);

  if (grid.visible ())
    display_obj (&grid);

  for (int i = 0; i < fieldlist.size (); i++)
    {
      const glgrib_field * fld = &fieldlist[i];
      if (! fld->isReady ())
        continue;
      if (fld->visible ())
        {
          const glgrib_program * program = fld->get_program ();
          program->use ();
          view.setMVP (program->matrixID);
          setLightShader (program->programID);
          fld->render (&view);
        }
    }

  float width = view.width, height = view.height;

  float ratio = width / height;

  if(1){
  glm::mat4 p = glm::ortho(0.0f, ratio, 0.0f, 1.0f, 0.1f, 100.0f);
  glm::mat4 v = glm::lookAt (glm::vec3 (+1.0f,0.0f,0.0f), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 mvp = p * v;
  str.render (mvp);
  }


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
  if (rotate_earth)
    view.opts.lon += 1.;
  if (light.rotate)
    light.lon -= 1.;


  if (movie && ((t - movie_time) > opts.scene.movie_wait))
    {
      bool advance = false;

      for (int i = 0; i < fieldlist.size (); i++)
        {
          glgrib_field fld;
          bool defined = opts.field[i].path.size () != 0;
	  if (defined)
            {
	      int size = opts.field[i].path.size ();
	      advance = movie_index < size;
	      int slot = advance ? movie_index : size-1;
              fld.init (opts.field[i], slot);
	      fieldlist[i] = fld;
	    }
	}

      if (advance)
        movie_index++;
      else
        movie_index = 0;

      movie_time = t;
    }
}

void glgrib_scene::init (const glgrib_options & o)
{
  opts = o;

  if (opts.landscape.path != "")
    landscape.init (opts.landscape);

  if (opts.grid.resolution)
    grid.init (opts.grid);

  if (opts.coastlines.path != "")
    coastlines.init (opts.coastlines);

  light = opts.scene.light;

  view.opts = opts.camera;

  for (int i = 0; i < opts.field.size (); i++)
    {
      glgrib_field fld;
      bool defined = opts.field[i].path.size () != 0;

      if (defined)
        fld.init (opts.field[i]);

      fieldlist.push_back (fld);

      if (defined)
        setCurrentFieldRank (i);

    }

  font = new_glgrib_font_ptr ();

  font->init (std::string ("alpha-array-08.bmp"));
  str.init (font, std::string ("ABC"), 0.0f, 0.0f, 0.05f, glgrib_string::SW);

}

