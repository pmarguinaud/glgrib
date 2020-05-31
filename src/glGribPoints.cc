#include "glGribPoints.h"
#include "glGribTrigonometry.h"
#include "glGribBitmap.h"

#include <iostream>
#include <algorithm>
#include <stdlib.h>


void glGrib::Points::clear ()
{
  if (isReady ())
    {
      VAID.clear ();
      glGrib::Object::clear ();
    }
}

void glGrib::Points::setupVertexAttributes () const
{
  glGrib::Program * program = glGrib::Program::load ("POINTS");
  auto attr = program->getAttributeLocation ("aPos");

  d.llsbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (attr); 

  glVertexAttribPointer (attr, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glVertexAttribDivisor (attr, 1);
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

  d.llsbuffer = glGrib::OpenGLBufferPtr<float> (lls.size (), lls.data ());

  d.p = glGrib::Palette (d.opts.palette, d.min, d.max);

  setReady ();
}

void glGrib::Points::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  render (view, light, 0, d.len);
}

void glGrib::Points::render (const glGrib::View & view, const glGrib::OptionsLight &, const int offset, const int length) const
{
  if (! isReady ())
    return;

  float length10 = view.pixelToDistAtNadir (10);

  glGrib::Program * program = glGrib::Program::load ("POINTS");
  program->use ();

  program->set ("scale0", d.opts.scale);
  program->set ("length10", length10);
  program->set ("valmin", d.min);
  program->set ("valmax", d.max);
  program->set ("ratio", view.getRatio ());

  program->set ("palmin", d.opts.palette.min);
  program->set ("palmax", d.opts.palette.max);
  program->set ("pointSiz", d.opts.size.value);

  program->set ("lpointSiz", d.opts.size.variable.on ? 1 : 0);

  if (d.opts.palette.name != "none")
    {
      d.p.set (program);
      program->set ("lcolor0", false);
    }
  else
    {
      program->set ("color0", d.opts.color);
      program->set ("lcolor0", true);
    }

  view.setMVP (program);

  VAID.bind ();
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
  glDrawElementsInstancedBaseInstance 
    (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, length, offset);
  VAID.unbind ();

  view.delMVP (program);

}



