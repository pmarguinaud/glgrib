#include "glgrib_points.h"
#include "glgrib_bitmap.h"

#include <iostream>
#include <algorithm>
#include <stdlib.h>


glgrib_points::~glgrib_points ()
{
  clear ();
}

void glgrib_points::clear ()
{
  if (ready)
    {
      glDeleteVertexArrays (1, &VertexArrayID);
    }
}

glgrib_points & glgrib_points::operator= (const glgrib_points & points)
{
  if (this != &points)
    {   
      clear (); 
      if (points.ready)
        {
          d = points.d;
          setupVertexAttributes (); 
          ready = true;
        }
    }   
}

void glgrib_points::setupVertexAttributes ()
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  d.llsbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 

  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glVertexAttribDivisor (0, 1);

  glBindVertexArray (0);
}

void glgrib_points::setup (const glgrib_options_points & o, 
		          const std::vector<float> & lon, 
                          const std::vector<float> & lat, 
                          const std::vector<float> & val)
{
  d.opts = o;

  const double deg2rad = M_PI / 180.0;

  d.len = lon.size ();
  std::vector<float> lls;
  lls.reserve (3 * d.len);

  d.min = *std::min_element (std::begin (val), std::end (val));
  d.max = *std::max_element (std::begin (val), std::end (val));

  for (int i = 0; i < d.len; i++)
    {
      lls.push_back (lon[i] * deg2rad);
      lls.push_back (lat[i] * deg2rad);
      lls.push_back (val[i]);
    }

  d.llsbuffer = new_glgrib_opengl_buffer_ptr (lls.size () * sizeof (float), lls.data ());

  if (d.opts.palette.min == glgrib_palette::defaultMin)
    d.opts.palette.min = d.min;
  if (d.opts.palette.max == glgrib_palette::defaultMax)
    d.opts.palette.max = d.max;

  setupVertexAttributes ();
  ready = true;
}

void glgrib_points::render (const glgrib_view & view) const
{
  if (! ready)
    return;

  float length = view.pixel_to_dist_at_nadir (10);

  glgrib_program * program = glgrib_program_load (glgrib_program::POINTS);
  program->use ();

  float scale0[3] = {d.opts.scale, d.opts.scale, d.opts.scale};
  program->set3fv ("scale0", scale0);
  program->set1f ("length10", length);
  program->set1f ("valmin", d.min);
  program->set1f ("valmax", d.max);

  program->set1f ("palmin", d.opts.palette.min);
  program->set1f ("palmax", d.opts.palette.max);
  program->set1f ("pointSiz", d.opts.size.value);

  program->set1i ("lpointSiz", d.opts.size.variable.on ? 1 : 0);

  if (d.opts.palette.name != "none")
    {
      glgrib_palette palette = glgrib_palette::create (d.opts.palette);
      palette.setRGBA255 (program->programID);
      program->set1f ("lcolor0", 0);
    }
  else
    {
      float color0[4] = {(float)d.opts.color.r/255.0f,(float)d.opts.color.g/255.0f,
                         (float)d.opts.color.b/255.0f,(float)d.opts.color.a/255.0f};
      program->set4fv ("color0", color0);
      program->set1f ("lcolor0", 1);
    }

  view.setMVP (program);

  glBindVertexArray (VertexArrayID);
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
  glBindVertexArray (0);

  view.delMVP (program);

}



