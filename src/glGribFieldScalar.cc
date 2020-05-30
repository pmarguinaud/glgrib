#include "glGribFieldScalar.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

void glGrib::FieldScalar::clear ()
{
  scalar.clear ();
  points.clear ();
  glGrib::Field::clear ();
}


glGrib::FieldScalar * glGrib::FieldScalar::clone () const
{
  glGrib::FieldScalar * fld = new glGrib::FieldScalar ();
  *fld = *this;
  return fld;
}

template <int N>
void glGrib::FieldScalar::field_t<N>::setupVertexAttributes () const
{
  switch (field->opts.scalar.pack.bits)
    {
      case  8:
        setupVertexAttributes_typed <unsigned char > ();
      break;
      case 16:
        setupVertexAttributes_typed <unsigned short> ();
      break;
      case 32:
        setupVertexAttributes_typed <unsigned int  > ();
      break;
      default:
        throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                  std::to_string (field->opts.scalar.pack.bits));
    }
}

template <>
std::string glGrib::FieldScalar::scalar_t::getProgramName () const
{
  return "SCALAR";
}

template <>
std::string glGrib::FieldScalar::points_t::getProgramName () const
{
  return "SCALAR_POINTS";
}

template <>
template <typename T>
void glGrib::FieldScalar::scalar_t::setupVertexAttributes_typed () const
{
  auto program = getProgram ();

  // Coordinates
  field->geometry->bindCoordinates (program->getAttributeLocation ("vertexLonLat"));

  // Values
  auto vattr = program->getAttributeLocation ("vertexVal");
  field->colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr); 
  bool fixed = field->palette.fixed ();
  glVertexAttribPointer (vattr, 1, getOpenGLType<T> (), fixed ? GL_FALSE : GL_TRUE, 0,  nullptr); 

  // Height
  field->bindHeight <T> (program->getAttributeLocation ("vertexHeight"));

  // MPI view
  auto mattr = program->getAttributeLocation ("vertexMPIView");
  if (field->opts.mpiview.on)
    {
      field->mpivbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (mattr); 
      glVertexAttribPointer (mattr, 3, GL_FLOAT, GL_FALSE, 0, nullptr); 
    }
  else
    {
      glDisableVertexAttribArray (mattr);
      glVertexAttrib3f (mattr, 0.0f, 0.0f, 0.0f);
    }

  // Triangles
  field->geometry->bindTriangles ();
}

template <>
template <typename T>
void glGrib::FieldScalar::points_t::setupVertexAttributes_typed () const
{
  auto program = getProgram ();

  // Coordinates
  auto pattr = program->getAttributeLocation ("vertexLonLat");
  field->geometry->bindCoordinates (pattr);
  glVertexAttribDivisor (pattr, 1);
  
  // Values
  auto vattr = program->getAttributeLocation ("vertexVal");
  field->colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr); 
  glVertexAttribPointer (vattr, 1, getOpenGLType<T> (), GL_TRUE, 0, nullptr); 
  glVertexAttribDivisor (vattr, 1);

  // Height
  auto hattr = program->getAttributeLocation ("vertexHeight");
  field->bindHeight <T> (hattr);
  glVertexAttribDivisor (hattr, 1);
  
}

void glGrib::FieldScalar::setup (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  opts = o;
  switch (opts.scalar.pack.bits)
    {
      case  8:
        setup<unsigned char > (ld, o, slot);
      break;
      case 16:
        setup<unsigned short> (ld, o, slot);
      break;
      case 32:
        setup<unsigned int  > (ld, o, slot);
      break;
      default:
        throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                  std::to_string (opts.scalar.pack.bits));
    }
}

template <typename T>
void glGrib::FieldScalar::setup (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  opts = o;

  glGrib::FieldMetadata meta1;

  glGrib::FieldFloatBufferPtr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glGrib::Palette (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glGrib::Geometry::load (ld, opts.path[int (slot)], opts.geometry);

  if (opts.hilo.on)
    setupHilo (data);

  colorbuffer = newGlgribOpenGLBufferPtr (geometry->getNumberOfPoints () * sizeof (T));

  auto col = colorbuffer->map<T> ();

  if (palette.fixed ())
    {
      const float * v = data->data ();
      const int n = geometry->getNumberOfPoints ();
      for (int i = 0; i < n; i++)
        {
          if (v[i] == meta1.valmis)
            col[i] = 0.0;
          else
            col[i] = static_cast<float> (palette.getColorIndex (v[i]));
        }
    }
  else
    {
      pack<T>  (data->data (), geometry->getNumberOfPoints (), meta1.valmin, 
                meta1.valmax, meta1.valmis, col.address ());
    }

  loadHeight <T> (colorbuffer, ld);

  if (opts.mpiview.on)
    setupMpiView (ld, o, slot);

  if (opts.no_value_pointer.on)
    values.push_back (newGlgribFieldFloatBufferPtr ((float*)nullptr));
  else
    values.push_back (data);

  setReady ();
}

void glGrib::FieldScalar::setupMpiView (glGrib::Loader * ld, const glGrib::OptionsField & o, float slot)
{
  int size = geometry->getNumberOfPoints ();

  glGrib::FieldMetadata mpiview_meta;
  glGrib::FieldFloatBufferPtr mpiview;

  ld->load (&mpiview, opts.mpiview.path, opts.geometry, slot, &mpiview_meta, 1, 0);

  float * data = mpiview->data ();

  float pmax = *std::max_element (data, data + size);

  int max = static_cast<int> (pmax);

  glm::vec3 Disp[max];
  glm::vec2 Disl[max];
  int count[max];

  for (int i = 0; i < max; i++)
    {
      Disp[i] = glm::vec3 (0.0f, 0.0f, 0.0f);
      count[i] = 0;
    }

  for (int i = 0; i < size; i++)
    {
      float lon, lat;
      geometry->index2latlon (i, &lat, &lon);

      int j = static_cast<int> ((*mpiview)[i]-1);

      Disp[j] += lonlat2xyz (glm::vec2 (lon, lat));
      count[j]++;
    }

  for (int i = 0; i < max; i++)
    Disl[i] = xyz2lonlat (glm::normalize (Disp[i] / static_cast<float> (count[i])));

  mpivbuffer = newGlgribOpenGLBufferPtr (3 * size * sizeof (float));

  auto mpiv = mpivbuffer->map<float> ();

  for (int i = 0; i < size; i++)
    {
      int j = (*mpiview)[i]-1;
      mpiv[3*i+0] = (*mpiview)[i];
      mpiv[3*i+1] = Disl[j].x;
      mpiv[3*i+2] = Disl[j].y;
    }

}

template <>
void glGrib::FieldScalar::scalar_t::render (const glGrib::View & view) const
{
  glGrib::Program * program = getProgram ();
  program->set ("smoothed", field->opts.scalar.smooth.on);
  
  if (field->opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  
  field->geometry->renderTriangles ();
  
  if (field->opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

template <>
void glGrib::FieldScalar::points_t::render (const glGrib::View & view) const
{
  glGrib::Program * program = getProgram ();
  float length = view.pixelToDistAtNadir (10);
  
  program->set ("length10", length);
  program->set ("pointSiz", field->opts.scalar.points.size.value);
  program->set ("lpointZoo", field->opts.scalar.points.size.variable.on);
  program->set ("factor", field->opts.scalar.points.size.factor.on);
  
  int numberOfPoints = field->geometry->getNumberOfPoints ();
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfPoints);
}

void glGrib::FieldScalar::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  float scale0 = opts.scale;

  if (opts.mpiview.on)
    scale0 = scale0 / (1.0f + opts.mpiview.scale);

  glGrib::Program * program = opts.scalar.points.on 
                            ? points.getProgram () 
                            : scalar.getProgram ();

  program->use ();

  view.setMVP (program);
  program->set (light);
  palette.set (program);
  program->set ("scale0", scale0);
  program->set ("valmin", getNormedMinValue ());
  program->set ("valmax", getNormedMaxValue ());
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("height_scale", opts.geometry.height.scale);
  program->set ("discrete", opts.scalar.discrete.on);
  program->set ("mpiview_scale", opts.mpiview.on ? opts.mpiview.scale : 0.0f);

  program->set ("RGBAM", opts.scalar.discrete.missing_color);
 
  geometry->setProgramParameters (program);

  int Nmax = 1;
  for (int i = 0; i < opts.scalar.pack.bits; i++)
    Nmax = Nmax * 2;

  program->set ("Nmax", Nmax-1);
    
  if (opts.scalar.points.on)
    points.VAID.render (view);
  else
    scalar.VAID.render (view);

  view.delMVP (program);

  renderHilo (view);
  
  if (opts.geometry.frame.on)
    renderFrame (view);

}


