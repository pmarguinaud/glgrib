#include "glGrib/FieldContour.h"
#include "glGrib/Program.h"
#include "glGrib/Palette.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Clear.h"
#include "glGrib/Contour.h"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <numeric>


namespace glGrib
{

namespace
{

float getLabelAngle (const std::vector<float> & lonlat, const std::vector<float> & length, int j, int width = 2)
{
  const glm::vec3 N = glm::vec3 (0.0f, 0.0f, 1.0f);

  glm::vec3 P = glGrib::lonlat2xyz (glm::vec2 (lonlat[2*j+0], lonlat[2*j+1]));
  std::vector<glm::vec3> V (2 * width);

  for (int i = -width; i < 0; i++)
    V[width+i+0] = glm::normalize (P - glGrib::lonlat2xyz (glm::vec2 (lonlat[2*(j+i)+0], lonlat[2*(j+i)+1])));

  for (int i = 1; i <= width; i++)
    V[width+i-1] = glm::normalize (glGrib::lonlat2xyz (glm::vec2 (lonlat[2*(j+i)+0], lonlat[2*(j+i)+1])) - P);

  auto A = glm::normalize (std::accumulate (V.begin (), V.end (), glm::vec3 (0.0f, 0.0f, 0.0f)));

  glm::vec3 u = glm::cross (N, P);
  glm::vec3 v = glm::cross (P, u);

  float x = glm::dot (u, A);
  float y = glm::dot (v, A);

  float angle = glGrib::rad2deg * atan2 (y, x);

  if (std::abs (angle - 180.0f) < std::abs (angle))
    angle = angle - 180.0f;

  return angle;
}

}

void FieldContour::isoline_t::setupLabels (const OptionsField & opts, 
                                           const isoline_data_t & iso_data)
{
  FontPtr font = getGlGribFontPtr (opts.contour.labels.font);
  char tmp[256];

  sprintf (tmp, opts.contour.labels.format.c_str (), d.level);

  std::string label = tmp;

  while (label.length () && label[0] == ' ')
    label = label.substr (1);

  // Start indices
  std::vector<int> ind;
  
  for (int i = 0; i < iso_data.size (); i++)
    if (iso_data.length[i] == 0.0f)
      ind.push_back (i);


  // Middle points
  std::vector<int> mnd (ind.size ());

#pragma omp parallel for
  for (size_t i = 1; i < ind.size (); i++)
    for (size_t j = ind[i-1]+1; j < static_cast<size_t> (ind[i]); j++)
      if (ind[i] - ind[i-1] > 5)
      if ((iso_data.length[j] - iso_data.length[ind[i-1]+1]) > 
          (iso_data.length[ind[i]-1] - iso_data.length[ind[i-1]+1]) / 2)
        {
          if (iso_data.length[ind[i]-1] - iso_data.length[ind[i-1]+1] > opts.contour.labels.distmin * deg2rad)
            mnd[i-1] = j;
          else
            mnd[i-1] = 0;
          break;
        }

  std::vector<int> jnd;
  std::copy_if (mnd.begin (), mnd.end (), std::back_inserter (jnd), [] (int i) { return i != 0; });
   
  int nlab = jnd.size ();

  std::vector<std::string> L (nlab);
  std::vector<float> X (nlab), Y (nlab), Z (nlab), A (nlab);

#pragma omp parallel for
  for (int i = 0; i < nlab; i++)
    {
      int j = jnd[i];
      lonlat2xyz (iso_data.lonlat[2*j+0], iso_data.lonlat[2*j+1], &X[i], &Y[i], &Z[i]);
      A[i] = getLabelAngle (iso_data.lonlat, iso_data.length, j);
      L[i] = label; 
    }

  d.labels.setup (font, L, X, Y, Z, A, opts.contour.labels.font.bitmap.scale, StringTypes::C);
  d.labels.setForegroundColor (opts.contour.labels.font.color.foreground);
  d.labels.setBackgroundColor (opts.contour.labels.font.color.background);

  d.labels.setScale (opts.scale * 1.001);

}

void FieldContour::isoline_t::setup (const OptionsField & opts, 
                                     float _level, size_t rank, const Palette & palette, 
                                     const FieldContour::isoline_data_t & data)
{
  d.level = _level;

  d.vertexbuffer   = OpenGLBufferPtr<float> (data.lonlat);
  if (opts.geometry.height.on)
    d.heightbuffer   = OpenGLBufferPtr<float> (data.height);
  d.distancebuffer = OpenGLBufferPtr<float> (data.length);
  d.size = data.size () - 1;
  
  if (rank < opts.contour.widths.size ())
    {
      d.wide = (d.width = opts.contour.widths[rank]);
    }
  if ((rank < opts.contour.lengths.size ()) && (rank < opts.contour.patterns.size ()))
    {
      d.dash = (d.length = opts.contour.lengths[rank]);
      for (size_t j = 0; j < opts.contour.patterns[rank].length (); j++)
        d.pattern.push_back (opts.contour.patterns[rank][j] == opts.contour.patterns[rank][0]);
    }
  
  d.color = palette.getColor (d.level);
  
  if (opts.contour.labels.on)
    setupLabels (opts, data);

}

void FieldContour::isoline_t::setupVertexAttributes () const
{
  Program * program = Program::load ("CONTOUR");

  d.vertexbuffer->bind (GL_ARRAY_BUFFER);
  
  for (int j = 0; j < 3; j++)
    {
      auto attr = program->getAttributeLocation (std::string ("vertexLonLat") 
                                               + std::to_string (j));
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 2, GL_FLOAT, GL_FALSE, 0, 
                             (const void *)(j * 2 * sizeof (float)));
      glVertexAttribDivisor (attr, 1);
    }
  
  if (d.heightbuffer != nullptr)
    {
      d.heightbuffer->bind (GL_ARRAY_BUFFER);
  
      for (int j = 0; j < 2; j++)
        {
          auto attr = program->getAttributeLocation (std::string ("vertexHeight") 
                                                   + std::to_string (j));
          glEnableVertexAttribArray (attr);
          glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, 
                                 (const void *)(j * sizeof (float)));
          glVertexAttribDivisor (attr, 1);
        }
    }
  else
    {
      for (int j = 0; j < 2; j++)
        {
          auto attr = program->getAttributeLocation (std::string ("vertexHeight") 
                                                   + std::to_string (j));
          glDisableVertexAttribArray (attr);
          glVertexAttrib1f (attr, 0.0f);
        }
    }
  
  d.distancebuffer->bind (GL_ARRAY_BUFFER);
  
  for (int j = 0; j < 2; j++)
    {
      auto attr = program->getAttributeLocation (std::string ("dist") 
                                               + std::to_string (j));
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
      glVertexAttribDivisor (attr, 1);
    }
  
}

void FieldContour::isoline_t::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("CONTOUR");

  program->set ("dash", d.dash);
  program->set ("color0", d.color);
  
  
  if (d.dash)
    {
      float length = view.pixelToDistAtNadir (this->d.length);
      program->set ("length", length);
      program->set ("N", static_cast<int> (d.pattern.size ()));
      program->set ("pattern", d.pattern);
    }
  if (d.wide)
    {
      float width = view.pixelToDistAtNadir (this->d.width);
      program->set ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, d.size);
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, d.size);
    }
}

FieldContour * FieldContour::clone () const
{
  FieldContour * fld = new FieldContour (Field::Privatizer ());
  *fld = *this;
  return fld;
}

void FieldContour::setup (const Field::Privatizer, Loader * ld, const OptionsField & o, float slot)
{
  opts = o;

  FieldMetadata meta1;
  BufferPtr<float> data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = Palette (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  setGeometry (Geometry::load (ld, opts.path[0], opts.geometry));

  const auto & geometry = getGeometry ();

  if (opts.hilo.on)
    setupHilo (data);

  BufferPtr<float> height = data;
  FieldMetadata meta_height = meta1;
  if ((opts.geometry.height.on) && (opts.geometry.height.path != ""))
    {
      GeometryPtr geometry_height = Geometry::load (ld, opts.geometry.height.path, opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Field and height have different geometries"));

      ld->load (&height, opts.geometry.height.path, opts.geometry, &meta_height);
    }

  std::vector<float> levels = opts.contour.levels;

  if (levels.size () == 0)
    {
      float min = glGrib::Palette::isDefaultMin (opts.contour.min) ? meta1.valmin : opts.contour.min;
      float max = glGrib::Palette::isDefaultMax (opts.contour.max) ? meta1.valmax : opts.contour.max;
      for (int i = 0; i < opts.contour.number; i++)
        levels.push_back (min + (i + 1) * (max - min) / (opts.contour.number + 1));
    }

  isoline_data_t iso_data[levels.size ()];

  class iso_helper
  {
  public:
    iso_helper (isoline_data_t * _iso, bool _height = false, const BufferPtr<float> * _H = nullptr,
                const FieldMetadata & _meta_height = FieldMetadata ())
     : iso (_iso), height (_height), H (_H), meta_height (_meta_height)
    {
    }
    void start ()
    {
      first = true;
    }
    void push (const glm::vec3 & xyzA, const glm::vec3 & xyzB, const glm::vec3 & xyzC,
               const int jgloA, const int jgloB, const int jgloC, const float a)
    {
      float X = (1 - a) * xyzA.x + a * xyzB.x;
      float Y = (1 - a) * xyzA.y + a * xyzB.y;
      float Z = (1 - a) * xyzA.z + a * xyzB.z;
      // Normalize
      float R = std::sqrt (X * X + Y * Y + Z * Z);
      X /= R; Y /= R; Z /= R;
      
      float V = 0.0f;
      
      if (height)
        {
          const BufferPtr<float> & h = *H;
          V = (h[jgloA] == meta_height.valmis) || (h[jgloB] == meta_height.valmis) 
            ? 0.0f 
           : ((1 - a) * h[jgloA] + a * h[jgloB] - meta_height.valmin) / (meta_height.valmax - meta_height.valmin);
        }
      
      iso->push (X, Y, Z, V);
      
      if (first)
        {
          first = false;
          xyzv_first[0] = X;
          xyzv_first[1] = Y;
          xyzv_first[2] = Z;
          xyzv_first[3] = V;
        }

    }
    void close (bool edge)
    {
      if (! edge)
        iso->push (xyzv_first[0], xyzv_first[1], xyzv_first[2], xyzv_first[3]);
      iso->push (0., 0., 0., 0.);
    }
  private:
    isoline_data_t * iso;
    bool height;
    const BufferPtr<float> * H;
    FieldMetadata meta_height;
    bool first = true;
    float xyzv_first[4];
  };

  class val_helper
  {
  public:
    val_helper (BufferPtr<float> & _data) : data (_data)
    {
    }
    float operator () (int jglo) const
    {
      return data[jglo];
    }
  private:
    BufferPtr<float> & data;
  };

  val_helper val (data);

#pragma omp parallel for
  for (size_t i = 0; i < levels.size (); i++)
    {
      iso_helper isoh (&iso_data[i], opts.geometry.height.on, &height, meta_height);
      Contour::processTriangles (geometry, levels[i], &isoh, val);
    }

  iso.resize (levels.size ());

  for (size_t i = 0; i < levels.size (); i++)
    iso[i].setup (opts, levels[i], i, palette, iso_data[i]);

  if (opts.no_value_pointer.on)
    clear (data);
  values.push_back (data);

  setReady ();

}

void FieldContour::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("CONTOUR");
  program->use ();

  view.setMVP (program);
  program->set ("scale0", opts.scale);
  program->set ("height_scale", opts.geometry.height.scale);

  for (const auto & is : iso)
    is.VAID.render (view, light);

  view.delMVP (program);

  renderHilo (view);

  if (opts.contour.labels.on)
    for (const auto & is : iso)
      is.d.labels.render (view, OptionsLight ());

}

}
