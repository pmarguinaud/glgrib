#include "glGrib/FieldVector.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Program.h"
#include "glGrib/Palette.h"
#include "glGrib/Clear.h"

#include <iostream>

namespace glGrib
{

FieldVector * FieldVector::clone () const
{
  FieldVector * fld = new FieldVector (Field::Privatizer ());
  *fld = *this;
  return fld;
}

template <>
void FieldVector::scalar_t::setupVertexAttributes () const
{
  Program * program = Program::load ("SCALAR");

  const auto & geometry = field->getGeometry ();

  // Norm/direction

  // Position
  geometry->bindCoordinates (program->getAttributeLocation ("vertexLonLat"));
  
  // Norm
  field->d.buffer_n->bind (GL_ARRAY_BUFFER);

  auto vattr = program->getAttributeLocation ("vertexVal");
  glEnableVertexAttribArray (vattr); 
  glVertexAttribPointer (vattr, 1, getOpenGLType<T> (), GL_TRUE, 
                         sizeof (T), nullptr); 

  geometry->bindTriangles ();

  field->bindHeight (program->getAttributeLocation ("vertexHeight"));

}

template <>
void FieldVector::vector_t::setupVertexAttributes () const
{
  Program * program = Program::load ("VECTOR");

  const auto & geometry = field->getGeometry ();

  // Vector

  // Position
  auto pattr = program->getAttributeLocation ("vertexLonLat");
  geometry->bindCoordinates (pattr);
  glVertexAttribDivisor (pattr, 1);  
  
  // Norm
  field->d.buffer_n->bind (GL_ARRAY_BUFFER);
  auto nattr = program->getAttributeLocation ("vertexVal_n");
  glEnableVertexAttribArray (nattr); 
  glVertexAttribPointer (nattr, 1, getOpenGLType<T> (), GL_TRUE, 
                         sizeof (T), nullptr); 
  glVertexAttribDivisor (nattr, 1);  


  // Direction
  field->d.buffer_d->bind (GL_ARRAY_BUFFER);
  auto dattr = program->getAttributeLocation ("vertexVal_d");
  glEnableVertexAttribArray (dattr); 
  glVertexAttribPointer (dattr, 1, getOpenGLType<T> (), GL_TRUE, 
                         sizeof (T), nullptr); 
  glVertexAttribDivisor (dattr, 1);  

  auto hattr = program->getAttributeLocation ("vertexHeight");
  field->bindHeight (hattr);
  glVertexAttribDivisor (hattr, 1);  

}

void FieldVector::setup 
  (const Field::Privatizer, Loader * ld, 
   const OptionsField & o)
{
  opts = o;

  FieldMetadata meta_u, meta_v;
  FieldMetadata meta_n, meta_d;

  BufferPtr<float> data_u, data_v;

  ld->load (&data_u, opts.path, opts.geometry, opts.slot, &meta_u, 2, 0);
  ld->load (&data_v, opts.path, opts.geometry, opts.slot, &meta_v, 2, 1);

  setGeometry (Geometry::load (ld, opts.path[int (2 * opts.slot)], opts.geometry));


  const auto & geometry = getGeometry ();
  const int numberOfPoints = geometry->getNumberOfPoints ();

  BufferPtr<float> data_n, data_d;

  Loader::uv2nd 
    (geometry, data_u, data_v, data_n, data_d, 
     meta_u, meta_v, meta_n, meta_d);

  d.buffer_n = OpenGLBufferPtr<T> (numberOfPoints);

  pack (data_n, numberOfPoints, 
        meta_n.valmin, meta_n.valmax, meta_n.valmis, 
        d.buffer_n);

  loadHeight (d.buffer_n, ld);

  d.buffer_d = OpenGLBufferPtr<T> (numberOfPoints);


  pack (data_d, numberOfPoints,
        meta_d.valmin, meta_d.valmax, meta_d.valmis, 
        d.buffer_d);

  const int npts = opts.vector.density;

  geometry->sample (d.buffer_d, 0, npts);

  meta.push_back (meta_n);
  meta.push_back (meta_d);

  if (opts.no_value_pointer.on)
    {
      clear (data_n);
      clear (data_d);
    }

  values.push_back (data_n);
  values.push_back (data_d);

  d.vscale = opts.vector.scale * (pi / npts) / (meta_n.valmax || 1.0f);

  palette = Palette (opts.palette, getNormedMinValue (), getNormedMaxValue ());
  
  setReady ();
}

template <>
void FieldVector::scalar_t::render
   (const View & view, 
    const OptionsLight & light) 
const
{
  const auto & opts = field->opts;
  const auto & palette = field->palette;
  float scale0 = opts.scale;

  std::vector<float> valmax = field->getMaxValue ();
  std::vector<float> valmin = field->getMinValue ();

  Program * program = Program::load ("SCALAR");
  program->use ();

  const auto & geometry = field->getGeometry ();

  geometry->setProgramParameters (program);

  view.setMVP (program);
  program->set (light);
  field->palette.set (program);

  scale0 *= 0.99;

  program->set ("scale0", scale0);
  program->set ("valmin", valmin[0]);
  program->set ("valmax", valmax[0]);
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("height_scale", opts.geometry.height.scale);
  program->set ("Nmax", 255);
  program->set ("discrete", false);
  program->set ("mpiview_scale", 0.0f);

  geometry->renderTriangles ();

  view.delMVP (program);
}


namespace
{

class arrow_t
{
public:
  arrow_t (int _numPoints, const std::vector<unsigned int> & _ind) 
      : ind (_ind), numPoints (_numPoints), fillable (true) {}
  arrow_t (int _numPoints, GLenum _linemode = GL_LINE_STRIP) 
      : numPoints (_numPoints), fillable (false), linemode (_linemode) {}
  arrow_t (const std::vector<unsigned int> & _ind) 
      : ind (_ind), numPoints (_ind.size ()), fillable (false), linemode (GL_LINES) {}
  // Shapes of arrows
  std::vector<unsigned int> ind = {0, 0, 0};
  // Number of points for each arrow kind
  int numPoints;
  bool fillable;
  GLenum linemode = GL_LINE_STRIP;
  void render (int numberOfPoints, bool fill) const
  {
    if (fillable && fill)
      glDrawElementsInstanced (GL_TRIANGLES, ind.size (), GL_UNSIGNED_INT, &ind[0], numberOfPoints);
    else if (linemode == GL_LINE_STRIP) 
      glDrawArraysInstanced (GL_LINE_STRIP, 0, numPoints, numberOfPoints); 
    else if (linemode == GL_LINES)
      glDrawElementsInstanced (GL_LINES, numPoints, GL_UNSIGNED_INT, &ind[0], numberOfPoints);
  }
};

}

template <>
void FieldVector::vector_t::render 
   (const View & view, 
    const OptionsLight & light) 
const
{
  const auto & opts = field->opts;
  const auto & palette = field->palette;

  std::vector<float> valmax = field->getMaxValue ();
  std::vector<float> valmin = field->getMinValue ();

// Display vectors

  Program * program = Program::load ("VECTOR");
  program->use ();

  const auto & geometry = field->getGeometry ();

  geometry->setProgramParameters (program);

  view.setMVP (program);
  program->set (light);
  field->palette.set (program);

  program->set ("scale0", opts.scale);
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("valmin_n", valmin[0]);
  program->set ("valmax_n", valmax[0]);
  program->set ("valmin_d", valmin[1]);
  program->set ("valmax_d", valmax[1]);
  program->set ("valmin", valmin[0]);
  program->set ("valmax", valmax[0]);
  program->set ("vscale", field->d.vscale);
  program->set ("height_scale", opts.geometry.height.scale);

  const arrow_t * arrow = nullptr;


  if (opts.vector.arrow.on)
    {
      program->set ("color0", opts.vector.arrow.color);
      program->set ("arrow_fixed", opts.vector.arrow.fixed.on);
      program->set ("arrow_min", opts.vector.arrow.min);
      program->set ("head", opts.vector.arrow.head_size);
     
      static const std::vector<arrow_t> arrows =
      {
        arrow_t (5),
        arrow_t (8, {2, 1, 6, 2, 3, 4}),
        arrow_t (8, {0, 2, 1}),
      };
     
      int kind = std::min (int (arrows.size () - 1), std::max (opts.vector.arrow.kind, 0));

      program->set ("arrow_kind", kind);

      arrow = &arrows[kind];
    }
  else if (opts.vector.barb.on)
    {
      program->set ("arrow_fixed", true);
      program->set ("color0", opts.vector.barb.color);

      program->set ("barbxleng", opts.vector.barb.length * std::cos (opts.vector.barb.angle * deg2rad));
      program->set ("barbyleng", opts.vector.barb.length * std::sin (opts.vector.barb.angle * deg2rad));
      program->set ("barbdleng", opts.vector.barb.dleng);
      program->set ("circthres", opts.vector.barb.circle.level);
      program->set ("barbthresmax", opts.vector.barb.levels.size ()-1);
      program->set ("barbthres", opts.vector.barb.levels);
      program->set ("pennthresmax", opts.vector.barb.pennant.levels.size ()-2);
      program->set ("pennthres", opts.vector.barb.pennant.levels);

      static const std::vector<arrow_t> arrows =
      {
        arrow_t ({
                   0, 1, // Line
                   2, 3, // First barb
                   4, 5, // Second barb
                   6, 7, // Third bard
                   8, 9, // Fourth barb
                   10, 11, // Fifth barb
                   12, 13, // Line
                   14, 15, 15, 16, 16, 17, // First pennant (triangle)
                   18, 19, 19, 20, 20, 21, // Second pennant 
                 }),
      };

      program->set ("arrow_kind", 3);
      arrow = &arrows[0];

    }

  const int numberOfPoints = geometry->getNumberOfPoints ();

  arrow->render (numberOfPoints, opts.vector.arrow.fill.on);

  view.delMVP (program);
}

void FieldVector::render 
  (const View & view, 
   const OptionsLight & light) 
const
{
  if (opts.vector.arrow.on || opts.vector.barb.on)
    vector.VAID.render (view, light);
  if (opts.vector.norm.on)
    scalar.VAID.render (view, light);

  const auto & geometry = this->getGeometry ();

  if (opts.geometry.frame.on && geometry->hasFrame ())
    this->renderFrame (view);
}

void FieldVector::reSample (const View & view)
{
  if (! values[1].allocated ())
    return; 

  const FieldMetadata & meta_n = meta[0];
  const FieldMetadata & meta_d = meta[1];

  const OptionsView & view_opts = view.getOptions ();
  float w = view_opts.distance * deg2rad * view_opts.fov;

  const int npts = 2 * opts.vector.density / w;

  const auto & geometry = getGeometry ();

  pack (values[1], geometry->getNumberOfPoints (), 
        meta_d.valmin, meta_d.valmax, meta_d.valmis, 
        d.buffer_d);

  geometry->sample (d.buffer_d, 0, npts);

  d.vscale = opts.vector.scale * (pi / npts) / (meta_n.valmax || 1.0f);
}

void FieldVector::reSize (const View & view)
{
  reSample (view);
}


}


