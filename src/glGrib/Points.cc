#include "glGrib/Points.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Bitmap.h"

#include <iostream>
#include <algorithm>


namespace glGrib
{

void Points::setupVertexAttributes () const
{
  Program * program = Program::load ("POINTS");
  auto attr = program->getAttributeLocation ("aPos");

  d.llsbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (attr); 

  glVertexAttribPointer (attr, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glVertexAttribDivisor (attr, 1);
}

void Points::setup 
  (const OptionsPoints & o, const std::vector<float> & lon, 
   const std::vector<float> & lat, const std::vector<float> & val)
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

  d.llsbuffer = OpenGLBufferPtr<float> (lls);

  d.p = Palette (d.opts.palette, d.min, d.max);

  const int n = lon.size ();
  points.resize (n);

#pragma omp parallel for
  for (int i = 0; i < n; i++)
    {
      float xyz[3];
      lonlat2xyz (deg2rad * lon[i], deg2rad * lat[i], &xyz[0], &xyz[1], &xyz[2]);
      points[i] = glGrib::KdTree<3>::Point (xyz);
    }

  tree.build ();

  setReady ();
}

void Points::render (const View & view, const OptionsLight & light) const
{
  render (view, light, 0, d.len);
}

void Points::render (const View & view, const OptionsLight &, const int offset, const int length) const
{
  if (! isReady ())
    return;

  float length10 = view.pixelToDistAtNadir (10);

  Program * program = Program::load ("POINTS");
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

}


