#include "glgrib_field_contour.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"
#include "glgrib_trigonometry.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <numeric>


glgrib_field_contour::glgrib_field_contour (const glgrib_field_contour & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

glgrib_field_contour * glgrib_field_contour::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_contour * fld = new glgrib_field_contour ();
  *fld = *this;
  return fld;
}

glgrib_field_contour & glgrib_field_contour::operator= (const glgrib_field_contour & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          iso = field.iso;
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}

void glgrib_field_contour::clear ()
{
  if (isReady ()) 
    for (auto & is : iso)
      glDeleteVertexArrays (1, &is.VertexArrayID);
  glgrib_field::clear ();
}

void glgrib_field_contour::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  for (auto & is : iso)
    {
      glGenVertexArrays (1, &is.VertexArrayID);
      glBindVertexArray (is.VertexArrayID);

      is.vertexbuffer->bind (GL_ARRAY_BUFFER);

      for (int j = 0; j < 3; j++)
        {
          glEnableVertexAttribArray (j);
          glVertexAttribPointer (j, 2, GL_FLOAT, GL_FALSE, 0, (const void *)(j * 2 * sizeof (float)));
          glVertexAttribDivisor (j, 1);
        }

      if (opts.geometry.height.on)
        {
          is.heightbuffer->bind (GL_ARRAY_BUFFER);

          for (int j = 0; j < 2; j++)
            {
              glEnableVertexAttribArray (3 + j);
              glVertexAttribPointer (3 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float)));
              glVertexAttribDivisor (3 + j, 1);
            }
        }
      else
        {
          for (int j = 0; j < 2; j++)
            {
              glDisableVertexAttribArray (3 + j);
              glVertexAttrib1f (3 + j, 0.0f);
            }
	}

      is.distancebuffer->bind (GL_ARRAY_BUFFER);

      for (int j = 0; j < 2; j++)
        {
          glEnableVertexAttribArray (5 + j); 
          glVertexAttribPointer (5 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
          glVertexAttribDivisor (5 + j, 1);
        }

      glBindVertexArray (0); 
    }
}

static
float getLabelAngle (const std::vector<float> & lonlat, const std::vector<float> & length, int j, int width = 2)
{
  const glm::vec3 N = glm::vec3 (0.0f, 0.0f, 1.0f);

  float lon = lonlat[2*j+0], lat = lonlat[2*j+1];

  glm::vec3 P = lonlat2xyz (glm::vec2 (lonlat[2*j+0], lonlat[2*j+1]));
  std::vector<glm::vec3> V (2 * width);

  for (int i = -width; i < 0; i++)
    V[width+i+0] = glm::normalize (P - lonlat2xyz (glm::vec2 (lonlat[2*(j+i)+0], lonlat[2*(j+i)+1])));

  for (int i = 1; i <= width; i++)
    V[width+i-1] = glm::normalize (lonlat2xyz (glm::vec2 (lonlat[2*(j+i)+0], lonlat[2*(j+i)+1])) - P);

  auto A = glm::normalize (std::accumulate (V.begin (), V.end (), glm::vec3 (0.0f, 0.0f, 0.0f)));

  glm::vec3 u = glm::cross (N, P);
  glm::vec3 v = glm::cross (P, u);

  float x = glm::dot (u, A);
  float y = glm::dot (v, A);

  float angle = rad2deg * atan2 (y, x);

  if (fabs (angle - 180.0f) < fabs (angle))
    angle = angle - 180.0f;

  return angle;
}


void glgrib_field_contour::setupLabels (isoline_t * iso, const isoline_data_t & iso_data)
{
  glgrib_font_ptr font = new_glgrib_font_ptr (opts.contour.labels.font);
  char tmp[256];

  sprintf (tmp, opts.contour.labels.format.c_str (), iso->level);

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
  for (int i = 0; i < ind.size ()-1; i++)
    for (int j = ind[i]+1; j < ind[i+1]; j++)
      if (ind[i+1] - ind[i] > 5)
      if ((iso_data.length[j] - iso_data.length[ind[i]+1]) > 
          (iso_data.length[ind[i+1]-1] - iso_data.length[ind[i]+1]) / 2)
        {
          if (iso_data.length[ind[i+1]-1] - iso_data.length[ind[i]+1] > opts.contour.labels.distmin * deg2rad)
            mnd[i] = j;
          else
            mnd[i] = 0;
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

  iso->labels.setup3D (font, L, X, Y, Z, A, opts.contour.labels.font.scale, 
                       glgrib_string::C);
  iso->labels.setForegroundColor (opts.contour.labels.font.color.foreground);
  iso->labels.setBackgroundColor (opts.contour.labels.font.color.background);
  iso->labels.setScaleXYZ (opts.scale * 1.001);

}

void glgrib_field_contour::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  glgrib_field_metadata meta1;
  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  if (opts.hilo.on)
    setupHilo (data);

  glgrib_field_float_buffer_ptr height = data;
  glgrib_field_metadata meta_height = meta1;
  if ((opts.geometry.height.on) && (opts.geometry.height.path != ""))
    {
      glgrib_geometry_ptr geometry_height = glgrib_geometry::load (ld, opts.geometry.height.path, opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Field and height have different geometries"));

      ld->load (&height, opts.geometry.height.path, opts.geometry, &meta_height);
    }

  int size = geometry->size ();
  float minval = *std::min_element (data->data (), data->data () + size);

  std::vector<float> levels = opts.contour.levels;

  if (levels.size () == 0)
    {
      float min = opts.contour.min == glgrib_options_contour::defaultMin ? meta1.valmin : opts.contour.min;
      float max = opts.contour.max == glgrib_options_contour::defaultMax ? meta1.valmax : opts.contour.max;
      for (int i = 0; i < opts.contour.number; i++)
        levels.push_back (min + (i + 1) * (max - min) / (opts.contour.number + 1));
    }

  isoline_data_t iso_data[levels.size ()];


  float * val = data->data ();

#pragma omp parallel for
  for (int i = 0; i < levels.size (); i++)
    {
      bool * seen = new bool[geometry->getNumberOfTriangles () + 1];

      for (int i = 0; i < geometry->getNumberOfTriangles () + 1; i++)
        seen[i] = false;
      seen[0] = true;

      // First visit edge triangles
      for (int it = 0; it < geometry->getNumberOfTriangles (); it++)
        if (geometry->triangleIsEdge (it))
          processTriangle (it, val, levels[i], height->data (), meta_height.valmin, 
          		   meta_height.valmax, meta_height.valmis, seen+1, &iso_data[i]);
  
      for (int it = 0; it < geometry->getNumberOfTriangles (); it++)
        processTriangle (it, val, levels[i], height->data (), meta_height.valmin, 
                         meta_height.valmax, meta_height.valmis, seen+1, &iso_data[i]);

      delete [] seen;
    }

  val = NULL;

  iso.resize (levels.size ());

  for (int i = 0; i < levels.size (); i++)
    {
      iso[i].level = levels[i];
      iso[i].vertexbuffer   = new_glgrib_opengl_buffer_ptr (iso_data[i].lonlat.size () * sizeof (float), 
                                                            iso_data[i].lonlat.data ());
      if (opts.geometry.height.on)
        iso[i].heightbuffer   = new_glgrib_opengl_buffer_ptr (iso_data[i].height.size () * sizeof (float), 
                                                              iso_data[i].height.data ());

      iso[i].distancebuffer = new_glgrib_opengl_buffer_ptr (iso_data[i].length.size () * sizeof (float), 
                                                            iso_data[i].length.data ());
      iso[i].size = iso_data[i].size () - 1;

      if (i < opts.contour.widths.size ())
        {
          iso[i].wide = (iso[i].width = opts.contour.widths[i]);
        }
      if ((i < opts.contour.lengths.size ()) && (i < opts.contour.patterns.size ()))
        {
          iso[i].dash = (iso[i].length = opts.contour.lengths[i]);
          for (int j = 0; j < opts.contour.patterns[i].length (); j++)
            iso[i].pattern.push_back (opts.contour.patterns[i][j] == opts.contour.patterns[i][0]);
        }

      iso[i].color = palette.getColor (levels[i]);

      if (opts.contour.labels.on)
        setupLabels (&iso[i], iso_data[i]);

      iso_data[i].clear ();


    }


  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    {
      values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
    }
  else
    {
      values.push_back (data);
    }


  setReady ();
}

void glgrib_field_contour::processTriangle (int it0, float * r, float r0, float * h, float hmin, float hmax, 
                                            float hmis, bool * seen, isoline_data_t * iso)
{
  int count = 0;
  bool cont = true;
  bool edge = false;
  int it = it0;
  int its[2];
  bool first = true;
  float xyzv_first[4];

  while (cont)
    {
      cont = false;

      if (seen[it])
        break;

      seen[it] = true;

      int jglo[3], itri[3];
      glm::vec3 xyz[3];

      geometry->getTriangleVertices (it, jglo);

      int n = 0;
      for (int i = 0; i < 3; i++)
        if (r[jglo[i]] < r0)
          n++;

      if ((n == 0) || (n == 3)) // 3 vertices have the same color
        break;

      geometry->getTriangleNeighbours (it, jglo, itri, xyz);

      if (count == 0) // First triangle; see if it is at the edge of the domain
        {
          int c = 0;
          for (int i = 0; i < 3; i++)
            {
              int iA = i, iB = (i + 1) % 3;
              int jgloA = jglo[iA], jgloB = jglo[iB];
              bool bA = r[jgloA] < r0, bB = r[jgloB] < r0;
              int itAB = itri[iA];
              if ((bA != bB) && (! seen[itAB]))
                c++;
            }
          edge = c != 2;
          if ((! edge) && geometry->triangleIsEdge (it))
            {
              seen[it] = false;
              return;
            }
        }

      // Find a way out of current triangle
      for (int i = 0; i < 3; i++)
        {
          int iA = i, iB = (i + 1) % 3;
          int jgloA = jglo[iA], jgloB = jglo[iB];
          bool bA = r[jgloA] < r0, bB = r[jgloB] < r0;
          int itAB = itri[iA];
          if ((bA != bB) && (! seen[itAB]))
            {
              bool lswap = jgloA > jgloB;
              if (lswap)
                {
                  std::swap (jgloA, jgloB);
                  std::swap (iA, iB);
                }
              float a = (r0 - r[jgloA]) / (r[jgloB] - r[jgloA]);
              // Coordinates of point
              float X = (1 - a) * xyz[iA].x + a * xyz[iB].x;
              float Y = (1 - a) * xyz[iA].y + a * xyz[iB].y;
              float Z = (1 - a) * xyz[iA].z + a * xyz[iB].z;
              // Normalize
              float R = sqrt (X * X + Y * Y + Z * Z);
              X /= R; Y /= R; Z /= R;

	      float V = 0.0f;

	      if (opts.geometry.height.on)
	        V = (h[jgloA] == hmis) || (h[jgloB] == hmis) ? 0.0f : ((1 - a) * h[jgloA] + a * h[jgloB] - hmin) / (hmax - hmin);

              iso->push (X, Y, Z, V);

	      if (first)
                {
                  first = false;
		  xyzv_first[0] = X;
		  xyzv_first[1] = Y;
		  xyzv_first[2] = Z;
		  xyzv_first[3] = V;
		}

              if (count < 2)
                its[count] = it;

              it = itAB;
              count++;
              cont = true;
              break;
            }
        }

      // Reset back seen array to false for first two triangles, so that contour lines be closed
      if ((count == 2) && (! edge))
        seen[its[0]] = false;
      if ((count == 3) && (! edge))
        seen[its[1]] = false;
    }

  if (count > 0)
    {
      if (! edge)
        iso->push (xyzv_first[0], xyzv_first[1], xyzv_first[2], xyzv_first[3]);
      iso->push (0., 0., 0., 0.);
    }

  return;
}

void glgrib_field_contour::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::CONTOUR);
  program->use ();
  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  const glgrib_palette & p = palette;

  view.setMVP (program);
  program->set3fv ("scale0", scale0);
  program->set1f ("height_scale", opts.geometry.height.scale);

  for (const auto & is : iso)
    {
      glBindVertexArray (is.VertexArrayID);

      program->set1i ("dash", is.dash);

      float color0[3] = {is.color.r/255.0f, 
                         is.color.g/255.0f, 
                         is.color.b/255.0f};
      program->set3fv ("color0", color0);


      if (is.dash)
        {
          float length = view.pixel_to_dist_at_nadir (is.length);
          program->set1f ("length", length);
          program->set1i ("N", is.pattern.size ());
          program->set1iv ("pattern", is.pattern.data (), is.pattern.size ());
        }
      if (is.wide)
        {
          float width = view.pixel_to_dist_at_nadir (is.width);
          program->set1f ("width", width);
          unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
          glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, is.size);
        }
      else
        {
          glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, is.size);
        }
      glBindVertexArray (0);
    }


  view.delMVP (program);

  renderHilo (view);

  if (opts.contour.labels.on)
    for (const auto & is : iso)
      is.labels.render (view);

}

glgrib_field_contour::~glgrib_field_contour ()
{
}


