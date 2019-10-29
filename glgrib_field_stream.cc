#include "glgrib_field_stream.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>


glgrib_field_stream::glgrib_field_stream (const glgrib_field_stream & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

glgrib_field_stream * glgrib_field_stream::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_stream * fld = new glgrib_field_stream ();
  *fld = *this;
  return fld;
}

glgrib_field_stream & glgrib_field_stream::operator= (const glgrib_field_stream & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          stream = field.stream;
          setupVertexAttributes ();
          setReady ();
        }
    }
}

void glgrib_field_stream::clear ()
{
  if (isReady ()) 
    glDeleteVertexArrays (1, &stream.VertexArrayID);
  glgrib_field::clear ();
}

void glgrib_field_stream::setupVertexAttributes ()
{
  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  glGenVertexArrays (1, &stream.VertexArrayID);
  glBindVertexArray (stream.VertexArrayID);

  stream.vertexbuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 3; j++)
    {
      glEnableVertexAttribArray (j);
      glVertexAttribPointer (j, 3, GL_FLOAT, GL_FALSE, 0, (const void *)(j * 3 * sizeof (float)));
      glVertexAttribDivisor (j, 1);
    }

  stream.normalbuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 2; j++)
    {
      glEnableVertexAttribArray (3 + j);
      glVertexAttribPointer (3 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float)));
      glVertexAttribDivisor (3 + j, 1);
    }

  stream.distancebuffer->bind (GL_ARRAY_BUFFER);

  for (int j = 0; j < 2; j++)
    {
      glEnableVertexAttribArray (5 + j); 
      glVertexAttribPointer (5 + j, 1, GL_FLOAT, GL_FALSE, 0, (const void *)(j * sizeof (float))); 
      glVertexAttribDivisor (5 + j, 1);
    }

  glBindVertexArray (0); 
}

void glgrib_field_stream::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;

  glgrib_field_metadata meta_u, meta_v;
  glgrib_field_metadata meta_n, meta_d;

  glgrib_field_float_buffer_ptr data_u = ld->load (opts.path, slot, &meta_u, 2, 0);
  glgrib_field_float_buffer_ptr data_v = ld->load (opts.path, slot, &meta_v, 2, 1);

  geometry = glgrib_geometry_load (ld, opts.path[0]);

  numberOfColors = 1;

  int size = geometry->size ();

  float normmax = 0.0f;
  for (int i = 0; i < size; i++)
    normmax = std::max (normmax, (*data_u)[i] * (*data_u)[i] + (*data_v)[i] * (*data_v)[i]);

  normmax = sqrt (normmax);


  // TODO : SETUP


  setupVertexAttributes ();

  setReady ();
}

void glgrib_field_stream::processTriangle (int it0, float * rx, float * ry, float r0, bool * seen, streamline_data_t * stream)
{
  // TODO
  return;
}

void glgrib_field_stream::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::STREAM);
  program->use ();
  float scale0[3] = {opts.scale, opts.scale, opts.scale};
  const glgrib_palette & p = palette;

  view.setMVP (program);
  program->set3fv ("scale0", scale0);


  glBindVertexArray (stream.VertexArrayID);

  float color0[3] = {255.0f/255.0f, 
                     255.0f/255.0f, 
                     255.0f/255.0f};
  program->set3fv ("color0", color0);

  bool wide = false;
  float Width = 0.1f;
  if (wide)
    {
      float width = view.pixel_to_dist_at_nadir (Width);
      program->set1f ("width", width);
      unsigned int ind[12] = {1, 0, 2, 3, 1, 2, 1, 3, 4, 1, 4, 5};
      glDrawElementsInstanced (GL_TRIANGLES, 12, GL_UNSIGNED_INT, ind, stream.size);
    }
  else
    {
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 2, stream.size);
    }

  glBindVertexArray (0);

}

glgrib_field_stream::~glgrib_field_stream ()
{
}


