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

glgrib_field_vector & glgrib_field_vector::operator= (const glgrib_field_vector & field)
{
  if (this != &field)
    {
      cleanup ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          buffer_n = field.buffer_n;
          buffer_d = field.buffer_d;
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
  buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 


  // Direction
  buffer_d->bind (GL_ARRAY_BUFFER);
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
  buffer_n->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (1, 1);  


  // Direction
  buffer_d->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (2); 
  glVertexAttribPointer (2, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, 
                         numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (2, 1);  


  geometry->elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

}

void glgrib_field_vector::init (const glgrib_options_field & opts, int slot)
{
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
        if (data_n[i] < meta_n.valmin)
          meta_n.valmin = data_n[i];
        if (data_n[i] > meta_n.valmax)
          meta_n.valmax = data_n[i];
      }
    else
      throw std::runtime_error ("Inconsistent domain definition for U/V");


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
  const int npts = 50;
  geometry->sample (col_d, 0, npts);

  buffer_n = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints 
                                               * sizeof (unsigned char), col_n);

  buffer_d = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->numberOfPoints 
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

  vscale = (M_PI / npts) / (meta_n.valmax || 1.0f);

  setReady ();
}

void glgrib_field_vector::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program (); 
  float scale0[3] = {dopts.scale, dopts.scale, dopts.scale};
  const glgrib_palette & p = dopts.palette;

  p.setRGBA255 (program->programID);

  std::vector<float> valmax = getMaxValue ();
  std::vector<float> valmin = getMinValue ();

  glUniform3fv (glGetUniformLocation (program->programID, "scale0"), 1, scale0);

  glUniform1f (glGetUniformLocation (program->programID, "valmin_n"), valmin[0]);
  glUniform1f (glGetUniformLocation (program->programID, "valmax_n"), valmax[0]);

  glUniform1f (glGetUniformLocation (program->programID, "valmin_d"), valmin[1]);
  glUniform1f (glGetUniformLocation (program->programID, "valmax_d"), valmax[1]);

  glUniform1f (glGetUniformLocation (program->programID, "valmin"), valmin[0]);
  glUniform1f (glGetUniformLocation (program->programID, "valmax"), valmax[0]);

  float palmax = p.hasMax () ? p.getMax () : valmax[0];
  float palmin = p.hasMin () ? p.getMin () : valmax[1];

  glUniform1f (glGetUniformLocation (program->programID, "palmin"), palmin);
  glUniform1f (glGetUniformLocation (program->programID, "palmax"), palmax);

  float color0[3] = {1.0f, 0.0f, 0.0f};
  glUniform3fv (glGetUniformLocation (program->programID, "color0"), 1, color0);

  glUniform1f (glGetUniformLocation (program->programID, "vscale"), vscale);

#ifdef UNDEF
//glgrib_field::render (view);
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);
#endif

  glBindVertexArray (VertexArrayIDvector);

  glDrawArraysInstanced (GL_LINE_STRIP, 0, 5, numberOfPoints); 


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
  buffer_d->bind (GL_ARRAY_BUFFER);
  unsigned char * col_d = (unsigned char *)glMapBufferRange (GL_ARRAY_BUFFER, 0, 
                                           numberOfPoints * sizeof (unsigned char), 
                                           GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

  float * data_d = values[1]->data ();
  const glgrib_field_metadata & meta_n = meta[0];
  const glgrib_field_metadata & meta_d = meta[1];

  const float deg2rad = M_PI / 180.0;
  float w = view.opts.distance * deg2rad * view.opts.fov;

  const int npts = 2 * 50 / w;

  for (int i = 0; i < geometry->numberOfPoints; i++)
    col_d[i] = 1 + (int)(254 * (data_d[i] - meta_d.valmin)
                 / (meta_d.valmax - meta_d.valmin));
  geometry->sample (col_d, 0, npts);

  vscale = (M_PI / npts) / (meta_n.valmax || 1.0f);

  glFlushMappedBufferRange (GL_ARRAY_BUFFER, 0, numberOfPoints * sizeof (unsigned char));
  glUnmapBuffer (GL_ARRAY_BUFFER);
}






