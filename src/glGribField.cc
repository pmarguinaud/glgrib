#include "glGribField.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribPalette.h"
#include "glGribFieldScalar.h"
#include "glGribFieldVector.h"
#include "glGribFieldContour.h"
#include "glGribFieldIsoFill.h"
#include "glGribFieldStream.h"
#include "glGribResolve.h"
#include "glGribSQLite.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>

#define DEF(T) \
template void glGrib::Field::unpack<T>  \
            (float *, const int, const float,   \
             const float, const float, const T *);  \
template void glGrib::Field::pack<T>  \
          (const float *, const int, const float,   \
             const float, const float, T *);  \
template void glGrib::Field::packUnpack<T>   \
          (const float *, float *, const int,   \
           const float, const float, const float); \
template void glGrib::Field::loadHeight <T> (glGrib::OpenGLBufferPtr, glGrib::Loader *); \
template void glGrib::Field::bindHeight <T> (int) const;

DEF (unsigned char)
DEF (unsigned short)
DEF (unsigned int)


namespace
{

int hiloCount (glGrib::const_GeometryPtr geometry, glGrib::FieldFloatBufferPtr data,
                int jglo0, int radius, bool lo)
{
  const float * val = data->data ();
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

      for (std::set<int>::iterator it = s1->begin (); it != s1->end (); it++)
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
                  if (val[jglo2] < val[jglo0])
                    return j-1;
                }
              else
                {
                  if (val[jglo2] > val[jglo0])
                    return j-1;
                }
              s2->insert (jglo2);
            }
        }


      for (std::set<int>::iterator it = s2->begin (); it != s2->end (); it++)
        seen.insert (*it);

      std::swap (s1, s2);
    }

  return radius;
}

}

void glGrib::Field::setupHilo (glGrib::FieldFloatBufferPtr data)
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
  int np = geometry->getNumberOfPoints ();

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

  glGrib::FontPtr font = newGlgribFontPtr (opts.hilo.font);

  hilo.setShared (false);
  hilo.setChange (true);
  hilo.setup3D (font, lhilo.L, lhilo.X, lhilo.Y, lhilo.Z, lhilo.A, 
                opts.hilo.font.scale, glGrib::String::C);
  hilo.setForegroundColor (opts.hilo.font.color.foreground);
  hilo.setScaleXYZ (opts.scale);

}

void glGrib::Field::setPaletteOptions (const glGrib::OptionsPalette & o) 
{ 
  palette = glGrib::Palette (o, getNormedMinValue (), getNormedMaxValue ());
}

void glGrib::Field::clear ()
{
  values.clear ();
  meta.clear ();
  hilo.clear ();
  VAID_frame.clear ();
  glGrib::World::clear ();
}

const glGrib::Palette & glGrib::Field::getPalette () const
{
  return palette;
}

const glGrib::OptionsField & glGrib::Field::getOptions () const 
{ 
  opts.palette = palette.getOptions ();
  return opts; 
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

glGrib::Field * glGrib::Field::create (const glGrib::OptionsField & opts, float slot, glGrib::Loader * ld)
{

  glGrib::Field * fld = nullptr;

  if (opts.path.size () == 0)
    return nullptr;

  try
    {

      glGrib::OptionsField opts1 = opts;
     
      if (opts.user_pref.on)
        getUserPref (&opts1, ld, slot);
     
     
      std::string type = opts1.type;
     
      std::transform (type.begin (), type.end (), type.begin (), ::toupper);
     
      if (type == "VECTOR")
        fld = new glGrib::FieldVector ();
      else if (type == "STREAM")
        fld = new glGrib::FieldStream ();
      else if (type == "CONTOUR")
        fld = new glGrib::FieldContour ();
      else if (type == "SCALAR")
        fld = new glGrib::FieldScalar ();
      else if (type == "ISOFILL")
        fld = new glGrib::FieldIsoFill ();
      else
        throw std::runtime_error (std::string ("Unknown field type : ") + type);
     
      fld->setup (ld, opts1, slot);
      fld->slot = slot;
    }
  catch (const std::runtime_error & e)
    {

      if (opts.fatal.on)
        throw e;

      std::cout << "Cannot load field : " << e.what () << std::endl;

      if (fld != nullptr)
        {
          fld->clear ();
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

template <typename T>
void glGrib::Field::loadHeight (glGrib::OpenGLBufferPtr buf, glGrib::Loader * ld)
{
  if (opts.geometry.height.on)
    {
      if (opts.geometry.height.path == "")
        {
          heightbuffer = buf;
        }
      else
        {
          glGrib::GeometryPtr geometry_height = glGrib::Geometry::load (ld, opts.geometry.height.path, opts.geometry);

          if (! geometry_height->isEqual (*geometry))
            throw std::runtime_error (std::string ("Field and height have different geometries"));

          int size = geometry->getNumberOfPoints ();

          glGrib::FieldFloatBufferPtr data;
          glGrib::FieldMetadata meta;

          ld->load (&data, opts.geometry.height.path, opts.geometry, &meta);

          heightbuffer = newGlgribOpenGLBufferPtr (size * sizeof (T));

          T * height = (T *)heightbuffer->map (); 

	  pack<T> (data->data (), size, meta.valmin, meta.valmax, meta.valmis, height);

          heightbuffer->unmap (); 

        }
    }

}

template <typename T>
void glGrib::Field::bindHeight (int attr) const
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
    hilo.render (view);
}

template <typename T>
void glGrib::Field::pack (const float * f, const int n, const float valmin, 
		         const float valmax, const float valmis, T * b)
{
  const T nmax = std::numeric_limits<T>::max () - 1;
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    if (f[i] == valmis)
      b[i] = 0;
    else
      b[i] = 1 + (T)round (nmax * (f[i] - valmin)/(valmax - valmin));
}

template <typename T>
void glGrib::Field::unpack (float * f, const int n, const float valmin, 
		           const float valmax, const float valmis, const T * b)
{
  const T nmax = std::numeric_limits<T>::max () - 1;
#pragma omp parallel for
  for (int i = 0; i < n; i++)
    if (b[i] == 0)
      f[i] = valmis;
    else
      f[i] = valmin + (valmax - valmin) * (b[i] - 1) / nmax;
}

template <typename T>
void glGrib::Field::packUnpack (const float * g, float * f, const int n, const float valmin, const float valmax, const float valmis)
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


void glGrib::Field::setupVertexAttributesFrame () const
{
  VAID_frame.setup ();
  VAID_frame.bind ();

  geometry->bindFrame (0);

  VAID_frame.unbind ();
}

void glGrib::Field::renderFrame (const glGrib::View & view) const
{
  glGrib::Program * program = glGrib::Program::load ("FRAME"); 

  program->use ();
  view.setMVP (program);
  
  program->set ("scale0", opts.scale * 1.001f);
  program->set ("colorb", opts.geometry.frame.color1);
  program->set ("colorw", opts.geometry.frame.color2);
  program->set ("dlon", opts.geometry.frame.dlon);
  program->set ("dlat", opts.geometry.frame.dlat);

  VAID_frame.bind ();

  if (opts.geometry.frame.width > 0.0f)
    {
      float width = view.pixelToDistAtNadir (opts.geometry.frame.width);
      program->set ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, 
                               geometry->getFrameNumberOfPoints ());
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, geometry->getFrameNumberOfPoints ());
    }

  VAID_frame.unbind ();
}




