#include "glGrib/Field.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Program.h"
#include "glGrib/Palette.h"

#include "glGrib/FieldScalar.h"
#include "glGrib/FieldVector.h"
#include "glGrib/FieldContour.h"
#include "glGrib/FieldIsoFill.h"
#include "glGrib/FieldStream.h"
#include "glGrib/FieldVertical.h"

#include "glGrib/Resolve.h"
#include "glGrib/SQLite.h"

#include <iostream>
#include <string>
#include <algorithm>

namespace
{

int hiloCount (glGrib::const_GeometryPtr geometry, glGrib::BufferPtr<float> data,
                int jglo0, int radius, bool lo)
{
  std::vector<int> neigh;
  std::set<int> seen;
  
  seen.insert (jglo0);

  std::set<int> seen1, * s2 = &seen1, seen2, * s1 = &seen2;

  // s1 : last inserted values
  // s2 : values being inserted
  s1->insert (jglo0);

  for (int j = 1; j < radius; j++)
    {

      s2->clear ();

      for (std::set<int>::iterator it = s1->begin (); it != s1->end (); ++it)
        {
          int jglo1 = *it;
          geometry->getPointNeighbours (jglo1, &neigh);
          for (size_t i = 0; i < neigh.size (); i++)
            {
              int jglo2 = neigh[i];
              if (seen.find (jglo2) != seen.end ())
                continue;
              if (lo)
                {
                  if (data[jglo2] < data[jglo0])
                    return j-1;
                }
              else
                {
                  if (data[jglo2] > data[jglo0])
                    return j-1;
                }
              s2->insert (jglo2);
            }
        }


      for (std::set<int>::iterator it = s2->begin (); it != s2->end (); ++it)
        seen.insert (*it);

      std::swap (s1, s2);
    }

  return radius;
}

}

void glGrib::Field::setupHilo (glGrib::BufferPtr<float> data)
{
  class hilo_t
  {
  public:
    std::vector<float> X, Y, Z, A;
    std::vector<std::string> L;
    void push (const std::string & l, float x, float y, float z, float a = 0.0f)
    {
      L.push_back (l);
      X.push_back (x); Y.push_back (y);
      Z.push_back (z); A.push_back (a);
    }
  };

  hilo_t lhilo;

  std::vector<int> neigh;

  const auto & geometry = getGeometry ();
  const int np = geometry->getNumberOfPoints ();

  const float radius = deg2rad * opts.hilo.radius;

#pragma omp parallel for
  for (int jglo = 0; jglo < np; jglo++)
    {
      float mesh = geometry->getLocalMeshSize (jglo);

      int r = static_cast<int> (radius / mesh);
      int chi = hiloCount (geometry, data, jglo, r, false);
      int clo = hiloCount (geometry, data, jglo, r, true);

      bool lhi = chi >= r;
      bool llo = clo >= r;

      if (lhi || llo)
        {
          float lon, lat;
          geometry->index2latlon (jglo, &lat, &lon);
	  float x, y, z;
          lonlat2xyz (lon, lat, &x, &y, &z);
#pragma omp critical
          {
            lhilo.push (lhi ? "H" : "L", x, y, z);
          }
        }
    }

  glGrib::FontPtr font = getGlGribFontPtr (opts.hilo.font);

  hilo.setup (font, lhilo.L, lhilo.X, lhilo.Y, lhilo.Z, lhilo.A, 
              opts.hilo.font.bitmap.scale, glGrib::StringTypes::C);
  hilo.setForegroundColor (opts.hilo.font.color.foreground);
  hilo.setScale (opts.scale);

}

void glGrib::Field::getUserPref (glGrib::OptionsField * opts, glGrib::Loader * ld, int slot)
{
  glGrib::OptionsField opts_sql = *opts;
  glGrib::OptionsField opts_ref;
  
  glGrib::FieldMetadata meta;
  ld->load (nullptr, opts_sql.path, opts->geometry, slot, &meta);

  glGrib::SQLite db (glGrib::Resolve ("glGrib.db"));
  
  std::string options;

  if (meta.CLNOMA != "")
    {
      glGrib::SQLite::stmt st = db.prepare ("SELECT options FROM CLNOMA2OPTIONS WHERE CLNOMA = ?;");
      st.bindall (&meta.CLNOMA);
      if (st.fetchRow (&options))
        goto found;
    }

  if ((meta.discipline != 255) && (meta.parameterCategory != 255) && (meta.parameterNumber != 255))
    {
      glGrib::SQLite::stmt st = db.prepare ("SELECT options FROM GRIB2OPTIONS WHERE discipline = ? "
		                           "AND parameterCategory = ? AND parameterNumber = ?;");
      st.bindall (&meta.discipline, &meta.parameterCategory, &meta.parameterNumber);
      if (st.fetchRow (&options))
        goto found;
    }
  
  return;

found:
  opts_sql.parseUnseen (options.c_str ());
  opts_sql.path = opts->path;
  *opts = opts_sql;
}

glGrib::Field * glGrib::Field::create (const glGrib::OptionsField & opts, glGrib::Loader * ld)
{
  glGrib::Field * fld = nullptr;

  if (opts.path.size () == 0)
    return nullptr;

  try
    {

      glGrib::OptionsField opts1 = opts;
     
      if (opts.user_pref.on)
        getUserPref (&opts1, ld, opts.slot);
     
     
      std::string type = opts1.type;
     
      std::transform (type.begin (), type.end (), type.begin (), ::toupper);
     
      const Field::Privatizer priv;

      if (type == "VECTOR")
        fld = new glGrib::FieldVector (priv);
      else if (type == "STREAM")
        fld = new glGrib::FieldStream (priv);
      else if (type == "CONTOUR")
        fld = new glGrib::FieldContour (priv);
      else if (type == "SCALAR")
        {
          switch (opts1.scalar.pack.bits)
            {
              case  8: fld = new glGrib::FieldScalar< 8> (priv); break;
              case 16: fld = new glGrib::FieldScalar<16> (priv); break;
              case 32: fld = new glGrib::FieldScalar<32> (priv); break;
              default:
                throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                          std::to_string (opts1.scalar.pack.bits));
            }
        }
      else if (type == "ISOFILL")
        fld = new glGrib::FieldIsoFill (priv);
      else if (type == "VERTICAL")
        fld = new glGrib::FieldVertical (priv);
      else
        throw std::runtime_error (std::string ("Unknown field type : ") + type);
     
      fld->setup (priv, ld, opts1);
    }
  catch (const std::runtime_error & e)
    {

      if (opts.fatal.on)
        throw e;

      std::cout << "Cannot load field : " << e.what () << std::endl;

      if (fld != nullptr)
        {
          delete fld;
	  fld = nullptr;
	}
    }

  return fld;
}

void glGrib::Field::saveOptions () const
{
  glGrib::OptionsField opts1, opts2;

  opts1 = opts;
  opts1.path.clear ();
  std::string options = opts1.asOption (opts2);

  glGrib::SQLite db (glGrib::Resolve ("glGrib.db"));

  if (meta[0].CLNOMA != "")
    {
      glGrib::SQLite::stmt st = db.prepare ("INSERT OR REPLACE INTO CLNOMA2OPTIONS (CLNOMA, options) VALUES (?, ?);");
      st.bindall (&meta[0].CLNOMA, &options);
      st.execute ();
    }
  if ((meta[0].discipline != 255) && (meta[0].parameterCategory != 255) && (meta[0].parameterNumber != 255))
    {
      glGrib::SQLite::stmt st = db.prepare ("INSERT OR REPLACE INTO GRIB2OPTIONS (discipline, "
                                           "parameterCategory, parameterNumber, options) VALUES (?, ?, ?, ?);");
      st.bindall (&meta[0].discipline, &meta[0].parameterCategory, &meta[0].parameterNumber, &options);
      st.execute ();
    }

}

template <int N>
void glGrib::FieldPacked<N>::loadHeight (glGrib::OpenGLBufferPtr<T> buf, glGrib::Loader * ld)
{
  if (! opts.geometry.height.on)
    return;
  
  if (opts.geometry.height.path == "")
    {
      heightbuffer = buf;
    }
  else
    {
      const auto & geometry = getGeometry ();
 
      glGrib::GeometryPtr geometry_height = glGrib::Geometry::load (ld, opts.geometry.height.path, opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Field and height have different geometries"));

      const int size = geometry->getNumberOfPoints ();

      glGrib::BufferPtr<float> data;
      glGrib::FieldMetadata meta;

      ld->load (&data, opts.geometry.height.path, opts.geometry, &meta);

      heightbuffer = glGrib::OpenGLBufferPtr<T> (size);

      pack (data, size, meta.valmin, meta.valmax, 
            meta.valmis, heightbuffer);

    }

}

template <int N>
void glGrib::FieldPacked<N>::createMask (glGrib::Loader * ld)
{
  if (! opts.scalar.mask.on)
    return;

  if (opts.scalar.mask.path != "")
    {
      const auto & geometry = getGeometry ();

      glGrib::GeometryPtr geometry_height = glGrib::Geometry::load (ld, opts.scalar.mask.path, opts.geometry);
      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Field and mask have different geometries"));

      const int size = geometry->getNumberOfPoints ();
      glGrib::FieldMetadata meta;
      BufferPtr<float> data;

      ld->load (&data, opts.scalar.mask.path, opts.geometry, &meta);
      maskbuffer = glGrib::OpenGLBufferPtr<float> (size);

      auto maskbuffer_ = maskbuffer->map ();
     
      for (int i = 0; i < size; i++)
        maskbuffer_[i] = data[i];
    }
  else
    {
      const auto & geometry = getGeometry ();
      const int size = geometry->getNumberOfPoints ();
     
      std::vector<float> mask (size);
     
      const std::vector<float> & X = opts.scalar.mask.x;
      const std::vector<float> & Y = opts.scalar.mask.y;
     
      size_t j = 0;
      for (int i = 0; i < size; i++)
        {
          float x = ((float)i)/((float)size);
          while (1)
            {
              if (X[j] <= x)
                break;
              j++;
              if (j > X.size ())
                throw std::runtime_error ("Out of bounds interpolation");
            }
          mask[i] = Y[j] + (x - X[j]) * (Y[j+1] - Y[j]) / (X[j+1] - X[j]);
        }
     
      for (int i = 0; i < size; i++)
        mask[i] = ((float)opts.scalar.mask.frames) * mask[i];
     
      maskbuffer = glGrib::OpenGLBufferPtr<float> (size);
      auto maskbuffer_ = maskbuffer->map ();
     
      if (opts.scalar.mask.rand.on)
        {
          for (int i = 0; i < size; i++)
            {
              int s = size - i;
              int j = rand () % s;
              int k = s - 1;
              maskbuffer_[i] = mask[j];
              mask[j] = mask[k];
            }
        }
      else
        {
          for (int i = 0; i < size; i++)
            maskbuffer_[i] = mask[i];
        }
    }
}

template <int N>
void glGrib::FieldPacked<N>::bindMask (int attr) const
{
  if (maskbuffer)
    {
      maskbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
  else
    {
      glDisableVertexAttribArray (attr);
      glVertexAttrib1f (attr, 0.0f);
    }
}

template <int N>
void glGrib::FieldPacked<N>::bindHeight (int attr) const
{
  if (heightbuffer)
    {
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 1, getOpenGLType<T> (), GL_TRUE, 0, nullptr);
    }
  else
    {
      glDisableVertexAttribArray (attr);
      glVertexAttrib1f (attr, 0.0f);
    }
}

void glGrib::Field::renderHilo (const glGrib::View & view) const
{
  if (opts.hilo.on)
    hilo.render (view, glGrib::OptionsLight ());
}

template <int N>
void glGrib::FieldPacked<N>::pack 
  (const glGrib::BufferPtr<float> & f, const int n, const float valmin, 
   const float valmax, const float valmis, glGrib::OpenGLBufferPtr<T> & bp)
{
  const T nmax = std::numeric_limits<T>::max () - 1;
  auto b = bp->map ();
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    if (f[i] == valmis)
      b[i] = 0;
    else
      b[i] = 1 + (T)round (nmax * (f[i] - valmin)/(valmax - valmin));
}

template <int N>
void glGrib::FieldPacked<N>::unpack 
  (glGrib::BufferPtr<float> & f, const int n, const float valmin, 
   const float valmax, const float valmis, const glGrib::OpenGLBufferPtr<T> & bp)
{
  const T nmax = std::numeric_limits<T>::max () - 1;
  auto b = bp->map ();
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    if (b[i] == 0)
      f[i] = valmis;
    else
      f[i] = valmin + (valmax - valmin) * (b[i] - 1) / nmax;
}

template <int N>
void glGrib::FieldPacked<N>::packUnpack 
  (const glGrib::BufferPtr<float> & g, glGrib::BufferPtr<float> & f, 
   const int n, const float valmin, const float valmax, const float valmis)
{
  const T nmax = std::numeric_limits<T>::max () - 1;
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    {
      if (g[i] != valmis)
        {
          T b = 1 + (T)round ((nmax - 1) * (g[i] - valmin)/(valmax - valmin));
          f[i] = valmin + (valmax - valmin) * (b - 1) / nmax;
        }
    }

}


void glGrib::Field::frame_t::render (const glGrib::View & view) const
{
  const auto & opts = field->opts;
  const auto & geom = field->getGeometry ();

  glGrib::Program * program = glGrib::Program::load ("FRAME"); 

  program->use ();
  view.setMVP (program);
  
  program->set ("scale0", opts.scale * 1.001f);
  program->set ("colorb", opts.geometry.frame.color1);
  program->set ("colorw", opts.geometry.frame.color2);
  program->set ("dlon", opts.geometry.frame.dlon);
  program->set ("dlat", opts.geometry.frame.dlat);

  if (opts.geometry.frame.width > 0.0f)
    {
      float width = view.pixelToDistAtNadir (opts.geometry.frame.width);
      program->set ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, 
                               geom->getFrameNumberOfPoints ());
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, geom->getFrameNumberOfPoints ());
    }

}

void glGrib::Field::update ()
{
  frameNumber++;
}


template class glGrib::FieldPacked< 8>;
template class glGrib::FieldPacked<16>;
template class glGrib::FieldPacked<32>;



