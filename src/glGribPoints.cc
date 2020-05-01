#include "glGribPoints.h"
#include "glGribTrigonometry.h"
#include "glGribBitmap.h"

#include <iostream>
#include <algorithm>
#include <stdlib.h>


glGrib::Points::~Points ()
{
  clear ();
}

void glGrib::Points::clear ()
{
  if (isReady ())
    {
      glDeleteVertexArrays (1, &VertexArrayID);
      glGrib::Object::clear ();
    }
}

glGrib::Points & glGrib::Points::operator= (const glGrib::Points & points)
{
  if (this != &points)
    {   
      clear (); 
      if (points.isReady ())
        {
          d = points.d;
          setupVertexAttributes (); 
          setReady ();
        }
    }   
  return *this;
}

void glGrib::Points::setupVertexAttributes ()
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  d.llsbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 

  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glVertexAttribDivisor (0, 1);

  glBindVertexArray (0);
}

void glGrib::Points::setup (const glGrib::OptionsPoints & o, 
		            const std::vector<float> & lon, 
                            const std::vector<float> & lat, 
                            const std::vector<float> & val)
{
  d.opts = o;

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

  d.llsbuffer = newGlgribOpenGLBufferPtr (lls.size () * sizeof (float), lls.data ());

  if (d.opts.palette.min == glGrib::Palette::defaultMin ())
    d.opts.palette.min = d.min;
  if (d.opts.palette.max == glGrib::Palette::defaultMax ())
    d.opts.palette.max = d.max;

  d.p = glGrib::Palette (d.opts.palette, d.opts.palette.min, d.opts.palette.max);

  setupVertexAttributes ();
  setReady ();
}

void glGrib::Points::render (const glGrib::View & view, const glGrib::OptionsLight &) const
{
  if (! isReady ())
    return;

  float length = view.pixelToDistAtNadir (10);

  glGrib::Program * program = glGrib::Program::load (glGrib::Program::POINTS);
  program->use ();

  program->set ("scale0", d.opts.scale);
  program->set ("length10", length);
  program->set ("valmin", d.min);
  program->set ("valmax", d.max);
  program->set ("ratio", view.getRatio ());

  program->set ("palmin", d.opts.palette.min);
  program->set ("palmax", d.opts.palette.max);
  program->set ("pointSiz", d.opts.size.value);

  program->set ("lpointSiz", d.opts.size.variable.on ? 1 : 0);

  if (d.opts.palette.name != "none")
    {
      d.p.bind (program);
      program->set ("lcolor0", false);
    }
  else
    {
      program->set ("color0", d.opts.color);
      program->set ("lcolor0", true);
    }

  view.setMVP (program);

  glBindVertexArray (VertexArrayID);
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, d.len);
  glBindVertexArray (0);

  view.delMVP (program);

}



