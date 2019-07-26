#include "glgrib_field_contour.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>


glgrib_field_contour::glgrib_field_contour (const glgrib_field_contour & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      cleanup ();
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
      cleanup ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          iso = field.iso;
          setupVertexAttributes ();
          setReady ();
        }
    }
}

void glgrib_field_contour::cleanup ()
{
  if (isReady ()) 
    for (int i = 0; i < iso.size (); i++)
      {
        glDeleteVertexArrays (1, &iso[i].VertexArrayID);
      }
  glgrib_field::cleanup ();
}

void glgrib_field_contour::setupVertexAttributes ()
{
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  for (int i = 0; i < iso.size (); i++)
    {
      glGenVertexArrays (1, &iso[i].VertexArrayID);
      glBindVertexArray (iso[i].VertexArrayID);

      iso[i].vertexbuffer->bind (GL_ARRAY_BUFFER);

      for (int j = 0; j < 3; j++)
        {
          glEnableVertexAttribArray (j);
          glVertexAttribPointer (j, 3, GL_FLOAT, GL_FALSE, 0, (const void *)(j * 3 * sizeof (float)));
          glVertexAttribDivisor (j, 1);
        }

      iso[i].normalbuffer->bind (GL_ARRAY_BUFFER);

      for (int j = 0; j < 2; j++)
        {
          glEnableVertexAttribArray (3 + j);
          glVertexAttribPointer (3 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float)));
          glVertexAttribDivisor (3 + j, 1);
        }

      iso[i].distancebuffer->bind (GL_ARRAY_BUFFER);

      for (int j = 0; j < 2; j++)
        {
          glEnableVertexAttribArray (5 + j); 
          glVertexAttribPointer (5 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
          glVertexAttribDivisor (5 + j, 1);
        }

      glBindVertexArray (0); 
    }
}

void glgrib_field_contour::init (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  glgrib_field_metadata meta1;
  glgrib_field_float_buffer_ptr data = ld->load (opts.path, slot, &meta1);
  meta.push_back (meta1);

  dopts.scale = opts.scale;

  geometry = glgrib_geometry_load (opts.path[0]);

  numberOfColors = 1;

  int size = geometry->size ();
  float maxval = *std::max_element (data->data (), data->data () + size);
  float minval = *std::min_element (data->data (), data->data () + size);

  std::vector<float> levels = opts.contour.levels;

  if (levels.size () == 0)
    for (int i = 0; i < opts.contour.number; i++)
      levels.push_back (minval + (i + 1) * (maxval - minval) / (opts.contour.number + 1));

  isoline_data_t iso_data[levels.size ()];


#pragma omp parallel for
  for (int i = 0; i < levels.size (); i++)
    {
      bool * seen = (bool *)malloc (sizeof (bool) * (geometry->numberOfTriangles + 1));

      for (int i = 0; i < geometry->numberOfTriangles + 1; i++)
        seen[i] = false;
      seen[0] = true;

      // First visit edge triangles
      for (int it = 0; it < geometry->numberOfTriangles; it++)
        if (geometry->triangleIsEdge (it))
          processTriangle (it, data->data (), levels[i], seen+1, &iso_data[i]);

      for (int it = 0; it < geometry->numberOfTriangles; it++)
        processTriangle (it, data->data (), levels[i], seen+1, &iso_data[i]);

      free (seen);
    }

  iso.resize (levels.size ());

  for (int i = 0; i < levels.size (); i++)
    {
      iso[i].level = levels[i];
      iso[i].vertexbuffer   = new_glgrib_opengl_buffer_ptr (iso_data[i].xyz.size () * sizeof (float), 
                                                            iso_data[i].xyz.data ());
      iso[i].normalbuffer   = new_glgrib_opengl_buffer_ptr (iso_data[i].drw.size () * sizeof (float), 
                                                            iso_data[i].drw.data ());
      iso[i].distancebuffer = new_glgrib_opengl_buffer_ptr (iso_data[i].dis.size () * sizeof (float), 
                                                            iso_data[i].dis.data ());
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

      int size = opts.contour.colors.size ();
      if (size > 0)
        iso[i].color = opts.contour.colors[i % size];

      iso_data[i].clear ();


    }


  setupVertexAttributes ();

  if (opts.no_value_pointer)
    {
      values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
    }
  else
    {
      values.push_back (data);
    }


  setReady ();
}

void glgrib_field_contour::processTriangle (int it0, float * r, float r0, bool * seen, isoline_data_t * iso)
{
  int count = 0;
  bool cont = true;
  bool edge = false;
  int it = it0;
  int its[2];
  static int II = 0;
  bool dbg = false;
  int ind_start = iso->size ();
  static FILE * fp = NULL;

  if (dbg && (fp == NULL))
    fp = fopen ("debug.txt", "w");


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


      if (dbg)
        fprintf (fp, " %4d : %4d %4d %4d : %4d %4d %4d : ", it, jglo[0], jglo[1], jglo[2],
                itri[0], itri[1], itri[2]);

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

              iso->push (X, Y, Z);

              if (dbg)
              fprintf (fp, " %6.2f %6.2f %6.2f | %6.2f %6.2f %6.2f | ", xyz[iA].x, xyz[iA].y, xyz[iA].z,
                                                                        xyz[iB].x, xyz[iB].y, xyz[iB].z);
              if (dbg)
              fprintf (fp, " %4d %4d %6.2f %6.2f %6.2f %4d\n", count, it, X, Y, Z, itAB);

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
        iso->push (iso->xyz[3*(ind_start+1)+0], iso->xyz[3*(ind_start+1)+1], iso->xyz[3*(ind_start+1)+2], 0.);
      iso->push (0., 0., 0., 0.);
      if (dbg)
        fprintf (fp, "--------------------------------- %d\n", II);
      if (dbg)
        fflush (fp);
      II++;
    }

  return;
}

void glgrib_field_contour::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::CONTOUR);
  program->use ();
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  view.setMVP (program);
  program->set3fv ("scale0", scale0);


  for (int i = 0; i < iso.size (); i++)
    {
      glBindVertexArray (iso[i].VertexArrayID);

      program->set1i ("dash", iso[i].dash);

      float color0[3] = {iso[i].color.r/255.0f, 
                         iso[i].color.g/255.0f, 
                         iso[i].color.b/255.0f};
      program->set3fv ("color0", color0);


      if (iso[i].dash)
        {
          float length = view.pixel_to_dist_at_nadir (iso[i].length);
          program->set1f ("length", length);
          program->set1i ("N", iso[i].pattern.size ());
          program->set1iv ("pattern", iso[i].pattern.data (), iso[i].pattern.size ());
        }
      if (iso[i].wide)
        {
          float width = view.pixel_to_dist_at_nadir (iso[i].width);
          program->set1f ("width", width);
          unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
          glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, iso[i].size);
        }
      else
        {
          glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, iso[i].size);
        }
      glBindVertexArray (0);
    }

}

glgrib_field_contour::~glgrib_field_contour ()
{
}


