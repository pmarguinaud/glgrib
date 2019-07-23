#include "glgrib_field_vector.h"
#include "glgrib_load.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glgrib_field_vector::glgrib_field_vector (const glgrib_field_vector & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      cleanup ();
      operator= (field);
    }
}

glgrib_field_vector * glgrib_field_vector::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_vector * fld = new glgrib_field_vector ();
  *fld = *this;
  return fld;
}

glgrib_field_vector & glgrib_field_vector::operator= (const glgrib_field_vector & other)
{
  if (this != &other)
    {
      cleanup ();
      if (other.isReady ())
        {
          glgrib_field::operator= (other);
	  d = other.d;
          d.buffer_d = new_glgrib_opengl_buffer_ptr (other.d.buffer_d);
          setupVertexAttributes ();
          setReady ();
        }
    }
}


void glgrib_field_vector::setupVertexAttributes ()
{
  // Norm/direction

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  // Position
  geometry->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  // Norm
  d.buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 


  // Direction
  d.buffer_d->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 


  geometry->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 


  // Vector

  glGenVertexArrays (1, &VertexArrayIDvector);
  glBindVertexArray (VertexArrayIDvector);

  // Position
  geometry->vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  glVertexAttribDivisor (0, 1);  
  
  // Norm
  d.buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (1, 1);  


  // Direction
  d.buffer_d->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (2, 1);  


  geometry->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

}

void glgrib_field_vector::init (const glgrib_options_field & o, int slot)
{
  opts = o;

  float * data_u, * data_v;
  glgrib_field_metadata meta_u, meta_v;
  float * data_n, * data_d;
  glgrib_field_metadata meta_n, meta_d;

  glgrib_load (opts.path[2*slot+0], &data_u, &meta_u);
  glgrib_load (opts.path[2*slot+1], &data_v, &meta_v);


  dopts.scale = opts.scale[slot];

  if (opts.palette[slot] == "default")
    dopts.palette = get_palette_by_meta (meta_u);
  else
    dopts.palette = get_palette_by_name (opts.palette[slot]);

  const_glgrib_geometry_ptr geom1 = glgrib_geometry_load (opts.path[2*slot+0]);

  const_glgrib_geometry_ptr geom2 = glgrib_geometry_load (opts.path[2*slot+1]);

  if (! geom1->isEqual (*geom2))
    {
      throw std::runtime_error (std::string ("Vector components have different geometries : ") 
                                + opts.path[2*slot+0] + ", " + opts.path[2*slot+1]);
    }


  geometry = geom1;


  data_n = (float *)malloc (geometry->numberOfPoints * sizeof (float));
  data_d = (float *)malloc (geometry->numberOfPoints * sizeof (float));

  const double rad2deg = 180.0 / M_PI;

  meta_n = meta_u; // TODO : handle this differently
  meta_d = meta_u;
  
  meta_n.valmin = std::numeric_limits<float>::max();
  meta_n.valmax = 0.0f;
  meta_d.valmin = -180.0f;
  meta_d.valmax = +180.0f;


  for (int i = 0; i < geometry->numberOfPoints; i++)
    if (data_u[i] == meta_u.valmis)
      {
        data_n[i] = meta_u.valmis;
        data_d[i] = meta_u.valmis;
      }
    else if (data_v[i] != meta_u.valmis)
      {
        data_n[i] = sqrt (data_u[i] * data_u[i] + data_v[i] * data_v[i]);
        data_d[i] = rad2deg * atan2 (data_v[i], data_u[i]);
      }
    else
      throw std::runtime_error ("Inconsistent domain definition for U/V");

  
  geometry->applyNormScale (data_n);

  for (int i = 0; i < geometry->numberOfPoints; i++)
    if (data_u[i] != meta_u.valmis)
      {
        if (data_n[i] < meta_n.valmin)
          meta_n.valmin = data_n[i];
        if (data_n[i] > meta_n.valmax)
          meta_n.valmax = data_n[i];
      }
    else
      {
        data_n[i] = meta_u.valmis;
      }

  free (data_u);
  free (data_v);

  numberOfColors = 1;

  unsigned char * col_n = (unsigned char *)malloc (numberOfColors 
                        * geometry->numberOfPoints * sizeof (unsigned char));
  unsigned char * col_d = (unsigned char *)malloc (numberOfColors 
                        * geometry->numberOfPoints * sizeof (unsigned char));


  geometry->applyUVangle (data_d);

  for (int i = 0; i < geometry->numberOfPoints; i++)
    {
      col_n[i] = 1 + (int)(254 * (data_n[i] - meta_n.valmin)
                   / (meta_n.valmax - meta_n.valmin));
      col_d[i] = 1 + (int)(254 * (data_d[i] - meta_d.valmin)
                   / (meta_d.valmax - meta_d.valmin));
    }

  float resolution = geometry->resolution ();
  const int npts = opts.vector.density;
  geometry->sample (col_d, 0, npts);

  d.buffer_n = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints 
                                               * sizeof (unsigned char), col_n);

  d.buffer_d = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints 
                                               * sizeof (unsigned char), col_d);

  free (col_n);
  free (col_d);

  meta.push_back (meta_n);
  meta.push_back (meta_d);

  numberOfPoints = geometry->numberOfPoints;
  numberOfTriangles = geometry->numberOfTriangles;

  setupVertexAttributes ();

  if (opts.no_value_pointer)
    {
      values.push_back (NULL);
      values.push_back (NULL);
      free (data_n);
      free (data_d);
    }
  else
    {
      values.push_back (new_glgrib_field_float_buffer_ptr (data_n));
      values.push_back (new_glgrib_field_float_buffer_ptr (data_d));
    }

  d.vscale = opts.vector.scale * (M_PI / npts) / (meta_n.valmax || 1.0f);

  setReady ();
}

void glgrib_field_vector::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program;
  
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};

  std::vector<float> valmax = getMaxValue ();
  std::vector<float> valmin = getMinValue ();

// Display vectors

  if (! opts.vector.hide_arrow)
    {
      program = glgrib_program_load (glgrib_program::GRADIENT_FLAT_SCALE_VECTOR);
      program->use ();
      view.setMVP (program);
      program->setLight (light);

      program->set3fv ("scale0", scale0);
      program->set1f ("valmin_n", valmin[0]);
      program->set1f ("valmax_n", valmax[0]);
      program->set1f ("valmin_d", valmin[1]);
      program->set1f ("valmax_d", valmax[1]);
      program->set1f ("valmin", valmin[0]);
      program->set1f ("valmax", valmax[0]);

      float color0[3] = {opts.vector.color.r/255.0f, opts.vector.color.g/255.0f, opts.vector.color.b/255.0f};
      program->set3fv ("color0", color0);
      program->set1f ("vscale", d.vscale);
      program->set1f ("head", opts.vector.head_size);

      glBindVertexArray (VertexArrayIDvector);
      glDrawArraysInstanced (GL_LINE_STRIP, 0, 5, numberOfPoints); 
    }

// Display vector norm

  if (! opts.vector.hide_norm)
    {
      program = glgrib_program_load (glgrib_program::GRADIENT_FLAT_SCALE_SCALAR);
      program->use ();
      view.setMVP (program);
      program->setLight (light);

      const glgrib_palette & p = dopts.palette;
      p.setRGBA255 (program->programID);

      for (int i = 0; i < 3; i++)
        scale0[i] *= 0.99;

      float palmax = p.hasMax () ? p.getMax () : valmax[0];
      float palmin = p.hasMin () ? p.getMin () : valmin[0];

      program->set3fv ("scale0", scale0);
      program->set1f ("valmin", valmin[0]);
      program->set1f ("valmax", valmax[0]);
      program->set1f ("palmin", palmin);
      program->set1f ("palmax", palmax);

      glBindVertexArray (VertexArrayID);
      glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
      glBindVertexArray (0);
    }

}

glgrib_field_vector::~glgrib_field_vector ()
{
}


void glgrib_field_vector::cleanup ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayIDvector);
  glgrib_field::cleanup ();
}

void glgrib_field_vector::reSample (const glgrib_view & view)
{
  d.buffer_d->bind (GL_ARRAY_BUFFER);
  unsigned char * col_d = (unsigned char *)glMapBufferRange (GL_ARRAY_BUFFER, 0, 
                                           numberOfPoints * sizeof (unsigned char), 
                                           GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

  float * data_d = values[1]->data ();
  const glgrib_field_metadata & meta_n = meta[0];
  const glgrib_field_metadata & meta_d = meta[1];

  const float deg2rad = M_PI / 180.0;
  float w = view.opts.distance * deg2rad * view.opts.fov;

  const int npts = 2 * opts.vector.density / w;

  for (int i = 0; i < geometry->numberOfPoints; i++)
    col_d[i] = 1 + (int)(254 * (data_d[i] - meta_d.valmin)
                 / (meta_d.valmax - meta_d.valmin));
  geometry->sample (col_d, 0, npts);

  d.vscale = opts.vector.scale * (M_PI / npts) / (meta_n.valmax || 1.0f);

  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, numberOfPoints * sizeof (unsigned char));
  glUnmapBuffer (GL_ARRAY_BUFFER);
}

void glgrib_field_vector::resize (const glgrib_view & view)
{
  reSample (view);
}





