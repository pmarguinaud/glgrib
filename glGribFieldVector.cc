#include "glGribFieldVector.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glGribFieldVector::glGribFieldVector (const glGribFieldVector & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

glGribFieldVector * glGribFieldVector::clone () const
{
  if (this == nullptr)
    return nullptr;
  glGribFieldVector * fld = new glGribFieldVector ();
  *fld = *this;
  return fld;
}

glGribFieldVector & glGribFieldVector::operator= (const glGribFieldVector & other)
{
  if (this != &other)
    {
      clear ();
      if (other.isReady ())
        {
          glGribField::operator= (other);
	  d = other.d;
          d.buffer_d = newGlgribOpenGLBufferPtr (other.d.buffer_d);
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}


void glGribFieldVector::setupVertexAttributes ()
{
  // Norm/direction

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  // Position
  geometry->bindCoordinates (0);
  
  // Norm
  d.buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (unsigned char), nullptr); 


  geometry->bindTriangles ();

  bindHeight <unsigned char> (2);

  glBindVertexArray (0); 


  // Vector

  glGenVertexArrays (1, &VertexArrayIDvector);
  glBindVertexArray (VertexArrayIDvector);

  // Position
  geometry->bindCoordinates (0);
  glVertexAttribDivisor (0, 1);  
  
  // Norm
  d.buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (unsigned char), nullptr); 
  glVertexAttribDivisor (1, 1);  


  // Direction
  d.buffer_d->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (unsigned char), nullptr); 
  glVertexAttribDivisor (2, 1);  

  bindHeight <unsigned char> (3);
  glVertexAttribDivisor (3, 1);  

  glBindVertexArray (0); 

}

void glGribFieldVector::setup (glGribLoader * ld, const glGribOptionsField & o, float slot)
{
  opts = o;

  glGribFieldMetadata meta_u, meta_v;
  glGribFieldMetadata meta_n, meta_d;

  glgrib_field_float_buffer_ptr data_u, data_v;
  ld->load (&data_u, opts.path, opts.geometry, slot, &meta_u, 2, 0);
  ld->load (&data_v, opts.path, opts.geometry, slot, &meta_v, 2, 1);

  geometry = glGribGeometry::load (ld, opts.path[int (2 * slot)], opts.geometry);


  glgrib_field_float_buffer_ptr data_n, data_d;

  glGribLoader::uv2nd (geometry, data_u, data_v, data_n, data_d, meta_u, meta_v, meta_n, meta_d);

  d.buffer_n = newGlgribOpenGLBufferPtr (geometry->getNumberOfPoints () * sizeof (unsigned char));
  unsigned char * col_n = (unsigned char *)d.buffer_n->map ();
  pack<unsigned char> (data_n->data (), geometry->getNumberOfPoints (), meta_n.valmin, meta_n.valmax, meta_n.valmis, col_n);
  col_n = nullptr;
  d.buffer_n->unmap ();

  loadHeight <unsigned char> (d.buffer_n, ld);

  d.buffer_d = newGlgribOpenGLBufferPtr (geometry->getNumberOfPoints () * sizeof (unsigned char));
  unsigned char * col_d = (unsigned char *)d.buffer_d->map ();
  pack<unsigned char> (data_d->data (), geometry->getNumberOfPoints (), meta_d.valmin, meta_d.valmax, meta_d.valmis, col_d);

  float resolution = geometry->resolution ();
  const int npts = opts.vector.density;
  geometry->sample (col_d, 0, npts);

  col_d = nullptr;
  d.buffer_d->unmap ();

  meta.push_back (meta_n);
  meta.push_back (meta_d);

  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    {
      values.push_back (newGlgribFieldFloatBufferPtr ((float*)nullptr));
      values.push_back (newGlgribFieldFloatBufferPtr ((float*)nullptr));
    }
  else
    {
      values.push_back (data_n);
      values.push_back (data_d);
    }

  d.vscale = opts.vector.scale * (pi / npts) / (meta_n.valmax || 1.0f);

  palette = glGribPalette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());
  
  setReady ();
}

void glGribFieldVector::renderNorms (const glGribView & view, 
                                       const glGribOptionsLight & light) 
const
{
  float scale0 = opts.scale;

  std::vector<float> valmax = getMaxValue ();
  std::vector<float> valmin = getMinValue ();

  glGribProgram * program = glGribProgram::load (glGribProgram::SCALAR);
  program->use ();

  geometry->setProgramParameters (program);

  view.setMVP (program);
  program->set (light);
  palette.setRGBA255 (program->programID);

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

  glBindVertexArray (VertexArrayID);
  geometry->renderTriangles ();
  glBindVertexArray (0);

  view.delMVP (program);

}

class arrow_t
{
public:
  arrow_t (int _numPoints, const std::vector<unsigned int> & _ind) 
      : ind (_ind), numPoints (_numPoints), fillable (true) {}
  arrow_t (int _numPoints, GLenum _linemode = GL_LINE_STRIP) 
      : numPoints (_numPoints), fillable (false), linemode (_linemode) {}
  arrow_t (const std::vector<unsigned int> & _ind) 
      : ind (_ind), fillable (false), linemode (GL_LINES), numPoints (_ind.size ()) {}
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

void glGribFieldVector::renderArrow (const glGribView & view, 
                                       const glGribOptionsLight & light) 
const
{
  std::vector<float> valmax = getMaxValue ();
  std::vector<float> valmin = getMinValue ();

// Display vectors

  glGribProgram * program = glGribProgram::load (glGribProgram::VECTOR);
  program->use ();

  geometry->setProgramParameters (program);

  view.setMVP (program);
  program->set (light);
  palette.setRGBA255 (program->programID);

  program->set ("scale0", opts.scale);
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("valmin_n", valmin[0]);
  program->set ("valmax_n", valmax[0]);
  program->set ("valmin_d", valmin[1]);
  program->set ("valmax_d", valmax[1]);
  program->set ("valmin", valmin[0]);
  program->set ("valmax", valmax[0]);
  program->set ("vscale", d.vscale);
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

      program->set ("barbxleng", opts.vector.barb.length * cos (opts.vector.barb.angle * deg2rad));
      program->set ("barbyleng", opts.vector.barb.length * sin (opts.vector.barb.angle * deg2rad));
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

  glBindVertexArray (VertexArrayIDvector);

  arrow->render (numberOfPoints, opts.vector.arrow.fill.on);

  glBindVertexArray (0);

  view.delMVP (program);
}

void glGribFieldVector::render (const glGribView & view, const glGribOptionsLight & light) const
{
  if (opts.vector.arrow.on || opts.vector.barb.on)
    renderArrow (view, light);
  if (opts.vector.norm.on)
    renderNorms (view, light);
}

glGribFieldVector::~glGribFieldVector ()
{
}


void glGribFieldVector::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayIDvector);
  glGribField::clear ();
}

void glGribFieldVector::reSample (const glGribView & view)
{
  unsigned char * col_d = (unsigned char *)d.buffer_d->map ();

  float * data_d = values[1]->data ();

  if (data_d == nullptr)
    return; 

  const glGribFieldMetadata & meta_n = meta[0];
  const glGribFieldMetadata & meta_d = meta[1];

  const glGribOptionsView & view_opts = view.getOptions ();
  float w = view_opts.distance * deg2rad * view_opts.fov;

  const int npts = 2 * opts.vector.density / w;

  pack<unsigned char> (data_d, geometry->getNumberOfPoints (), meta_d.valmin, meta_d.valmax, meta_d.valmis, col_d);

  geometry->sample (col_d, 0, npts);

  d.vscale = opts.vector.scale * (pi / npts) / (meta_n.valmax || 1.0f);

  d.buffer_d->unmap ();

}

void glGribFieldVector::resize (const glGribView & view)
{
  reSample (view);
}





