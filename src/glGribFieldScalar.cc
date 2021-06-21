#include "glGribFieldScalar.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"
#include "glGribClear.h"

#include <iostream>
#include <algorithm>

namespace glGrib
{

template <int N>
FieldScalar<N> * FieldScalar<N>::clone () const
{
  FieldScalar<N> * fld = new FieldScalar<N> (Field::Privatizer ());
  *fld = *this;
  return fld;
}

template <int N>
void FieldScalar<N>::scalar_t::setupVertexAttributes () const
{
  auto program = this->getProgram ();
  const auto & field = this->field;

  const auto & geometry = field->getGeometry ();

  // Coordinates
  geometry->bindCoordinates (program->getAttributeLocation ("vertexLonLat"));

  // Values
  auto vattr = program->getAttributeLocation ("vertexVal");
  field->colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr); 
  bool fixed = field->palette.fixed ();
  glVertexAttribPointer (vattr, 1, getOpenGLType<T> (), fixed ? GL_FALSE : GL_TRUE, 0,  nullptr); 

  // Height
  field->bindHeight (program->getAttributeLocation ("vertexHeight"));

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
  geometry->bindTriangles (field->opts.geometry.subgrid.on ? 1 : 0);
}

template <int N>
void FieldScalar<N>::points_t::setupVertexAttributes () const
{
  auto program = this->getProgram ();
  const auto & field = this->field;
  const auto & geometry = field->getGeometry ();

  // Coordinates
  auto pattr = program->getAttributeLocation ("vertexLonLat");
  geometry->bindCoordinates (pattr);
  glVertexAttribDivisor (pattr, 1);
  
  // Values
  auto vattr = program->getAttributeLocation ("vertexVal");
  field->colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (vattr); 
  glVertexAttribPointer (vattr, 1, getOpenGLType<T> (), GL_TRUE, 0, nullptr); 
  glVertexAttribDivisor (vattr, 1);

  // Height
  auto hattr = program->getAttributeLocation ("vertexHeight");
  field->bindHeight (hattr);
  glVertexAttribDivisor (hattr, 1);
  
}

static void widenRegion (const std::vector<float> & values, const std::vector<int> & radius,
                         const_GeometryPtr geometry, BufferPtr<float> & data) 
{
  const int size = *std::max_element (radius.begin (), radius.end ());

  const int sz = geometry->size ();
  const int nb = values.size ();

  std::set<int> halo1[nb], halo2[nb];
  
  std::vector<int> neigh;

  // Find halo : scan whole grid

#pragma omp parallel for private (neigh) 
  for (int jglo = 0; jglo < sz; jglo++) 
  for (int ib = 0; ib < nb; ib++)
    {
      if (data[jglo] != values[ib])
        continue;

      geometry->getPointNeighbours (jglo, &neigh);

      for (auto jglh : neigh)
        {
          if (data[jglh] == values[ib])
            continue;
#pragma omp critical
          halo1[ib].insert (jglh);
        }
    }
  
  // halo1 contains the previous halo
  // halo2 is filled with the next halo

  for (int iter = 0; iter < size; iter++)
  for (int ib = 0; ib < nb; ib++)
    {
      if (iter >= radius[ib % radius.size ()])
        continue;

      for (auto jglh : halo1[ib])
        {
          if (std::any_of (values.begin (), values.begin () + ib, [&] (float x) 
              { return x == data[jglh]; }))
            continue;
          data[jglh] = values[ib];
        }

      for (auto jglo : halo1[ib])
        {
          geometry->getPointNeighbours (jglo, &neigh);
          for (auto jglh : neigh)
            {
              if (data[jglh] == values[ib])
                continue;
              halo2[ib].insert (jglh);
            }
        }

      std::swap (halo1[ib], halo2[ib]);
      halo2[ib].clear ();

    }


}

template <int N>
void FieldScalar<N>::setup (const Field::Privatizer, Loader * ld, const OptionsField & o, float slot)
{
  auto & opts = this->opts;
  auto & palette = this->palette;

  opts = o;

  FieldMetadata meta1;

  BufferPtr<float> data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  this->meta.push_back (meta1);

  palette = Palette (opts.palette, this->getNormedMinValue (), this->getNormedMaxValue ());

  this->setGeometry (Geometry::load (ld, opts.path[int (slot)], opts.geometry));

  if (opts.hilo.on)
    this->setupHilo (data);

  const auto & geometry = this->getGeometry ();

  colorbuffer = OpenGLBufferPtr<T> (geometry->getNumberOfPoints ());


  if (palette.fixed ())
    {
      auto col = colorbuffer->map ();
      const int n = geometry->getNumberOfPoints ();
      for (int i = 0; i < n; i++)
        {
          if (data[i] == meta1.valmis)
            col[i] = 0.0;
          else
            col[i] = static_cast<float> (palette.getColorIndex (data[i]));
        }
    }
  else
    {

      if (opts.scalar.widen.on)
        widenRegion (opts.scalar.widen.values, opts.scalar.widen.radius, geometry, data);

      this->pack (data, geometry->getNumberOfPoints (), meta1.valmin, 
                  meta1.valmax, meta1.valmis, colorbuffer);
    }

  this->loadHeight (colorbuffer, ld);

  if (opts.mpiview.on)
    setupMpiView (ld, o, slot);

  if (opts.no_value_pointer.on)
    clear (data);

  this->values.push_back (data);

  this->setReady ();
}

template <int N>
void FieldScalar<N>::setupMpiView (Loader * ld, const OptionsField & o, float slot)
{
  auto & opts = this->opts;
  const auto & geometry = this->getGeometry ();
  const int size = geometry->getNumberOfPoints ();

  FieldMetadata mpiview_meta;
  BufferPtr<float> mpiview;

  ld->load (&mpiview, std::vector<OptionFieldRef>{opts.mpiview.path}, 
            opts.geometry, slot, &mpiview_meta, 1, 0);

  float pmax = mpiview_meta.valmax;

  int max = static_cast<int> (pmax);

  glm::vec3 Disp[max];
  glm::vec2 Disl[max];
  int count[max];

#pragma omp parallel for
  for (int mpi = 0; mpi < max; mpi++)
    {
      Disp[mpi] = glm::vec3 (0.0f, 0.0f, 0.0f);
      count[mpi] = 0;
    }

#pragma omp parallel for
  for (int jglo = 0; jglo < size; jglo++)
    {
      float lon, lat;
      geometry->index2latlon (jglo, &lat, &lon);

      int mpi = static_cast<int> (std::round (mpiview[jglo])) - 1;

      glm::vec3 d = lonlat2xyz (glm::vec2 (lon, lat));

#pragma omp critical
      {
        Disp[mpi] += d;
        count[mpi]++;   
      }
    }

#pragma omp parallel for
  for (int mpi = 0; mpi < max; mpi++)
    Disl[mpi] = xyz2lonlat (glm::normalize (Disp[mpi] / static_cast<float> (count[mpi])));

  mpivbuffer = OpenGLBufferPtr<float> (3 * size);

  auto mpiv = mpivbuffer->map ();

#pragma omp parallel for
  for (int jglo = 0; jglo < size; jglo++)
    {
      int mpi = static_cast<int> (std::round (mpiview[jglo]))-1;
      mpiv[3*jglo+0] = mpi + 1;
      mpiv[3*jglo+1] = Disl[mpi].x;
      mpiv[3*jglo+2] = Disl[mpi].y;
    }

}

template <int N>
void FieldScalar<N>::scalar_t::render (const View & view) const
{
  Program * program = this->getProgram ();
  const auto & field = this->field;
  const auto & geometry = field->getGeometry ();

  program->set ("smoothed", field->opts.scalar.smooth.on);
  
  if (field->opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  
  geometry->renderTriangles (field->opts.geometry.subgrid.on ? 1 : 0);
  
  if (field->opts.scalar.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

template <int N>
void FieldScalar<N>::points_t::render (const View & view) const
{
  Program * program = this->getProgram ();
  const auto & field = this->field;
  const auto & geometry = field->getGeometry ();

  float length = view.pixelToDistAtNadir (10);
  
  program->set ("length10", length);
  program->set ("pointSiz", field->opts.scalar.points.size.value);
  program->set ("lpointZoo", field->opts.scalar.points.size.variable.on);
  program->set ("factor", field->opts.scalar.points.size.factor.on);
  
  const int numberOfPoints = geometry->getNumberOfPoints ();
  unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
  glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfPoints);
}

template <int N>
void FieldScalar<N>::render (const View & view, const OptionsLight & light) const
{
  const auto & opts = this->opts;
  const auto & palette = this->palette;
  const auto & geometry = this->getGeometry ();
  float scale0 = opts.scale;

  if (opts.mpiview.on)
    scale0 = scale0 / (1.0f + opts.mpiview.scale);

  Program * program = opts.scalar.points.on 
                            ? points.getProgram () 
                            : scalar.getProgram ();

  program->use ();

  view.setMVP (program);
  program->set (light);
  program->set ("lreverse", opts.scalar.light.reverse.on);
  program->set ("breverse", opts.scalar.light.reverse.b);
  program->set ("creverse", opts.scalar.light.reverse.c);

  palette.set (program);
  program->set ("scale0", scale0);
  program->set ("valmin", this->getNormedMinValue ());
  program->set ("valmax", this->getNormedMaxValue ());
  program->set ("palmin", palette.getMin ());
  program->set ("palmax", palette.getMax ());
  program->set ("height_scale", opts.geometry.height.scale);
  program->set ("discrete", opts.scalar.discrete.on);
  program->set ("dinteger", opts.scalar.discrete.integer.on);
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

  this->renderHilo (view);
  
  if (opts.geometry.frame.on && geometry->hasFrame ())
    this->renderFrame (view);

}


template class FieldScalar< 8>;
template class FieldScalar<16>;
template class FieldScalar<32>;

}
