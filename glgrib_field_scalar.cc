#include "glgrib_field_scalar.h"
#include "glgrib_trigonometry.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>

glgrib_field_scalar::glgrib_field_scalar (const glgrib_field_scalar & field)
{
  if (field.isReady ())
    {
      // Cleanup already existing VAOs
      clear ();
      operator= (field);
    }
}

void glgrib_field_scalar::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayIDpoints);
  glgrib_field::clear ();
}


glgrib_field_scalar * glgrib_field_scalar::clone () const
{
  if (this == NULL)
    return NULL;
  glgrib_field_scalar * fld = new glgrib_field_scalar ();
  *fld = *this;
  return fld;
}

glgrib_field_scalar & glgrib_field_scalar::operator= (const glgrib_field_scalar & field)
{
  if (this != &field)
    {
      clear ();
      if (field.isReady ())
        {
          glgrib_field::operator= (field);
          setupVertexAttributes ();
          setReady ();
        }
    }
  return *this;
}

void glgrib_field_scalar::setupVertexAttributes ()
{
  switch (opts.scalar.pack.bits)
    {
      case  8:
        setupVertexAttributes <unsigned char > ();
      break;
      case 16:
        setupVertexAttributes <unsigned short> ();
      break;
      case 32:
        setupVertexAttributes <unsigned int  > ();
      break;
      default:
        throw std::runtime_error (std::string ("Wrong number of bits for packing field: ") +
                                  std::to_string (opts.scalar.pack.bits));
    }
}

template <typename T>
void glgrib_field_scalar::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->bindCoordinates (0);

  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 

  glVertexAttribPointer (1, 1, getOpenglType<T> (), GL_TRUE, 0,  NULL); 

  geometry->bindTriangles ();

  bindHeight <T> (2);

  if (opts.mpiview.on)
    {
      mpivbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (3); 
      glVertexAttribPointer (3, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
//    glVertexAttribDivisor (3, 1);
    }
  else
    {
      glDisableVertexAttribArray (3);
      glVertexAttrib3f (3, 0.0f, 0.0f, 0.0f);
    }


  glBindVertexArray (0); 

  // Points
  glGenVertexArrays (1, &VertexArrayIDpoints);
  glBindVertexArray (VertexArrayIDpoints);

  geometry->bindCoordinates (0);
  glVertexAttribDivisor (0, 1);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, 1, getOpenglType<T> (), GL_TRUE, 0, NULL); 
  glVertexAttribDivisor (1, 1);

  if (heightbuffer)
    {
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (2);
      glVertexAttribPointer (2, 1, getOpenglType<T> (), GL_TRUE, 0, NULL);
      glVertexAttribDivisor (2, 1);
    }
  else
    {
      glDisableVertexAttribArray (2);
      glVertexAttrib1f (2, 0.0f);
    }


  glBindVertexArray (0); 

  if (opts.geometry.frame.on)
    setupVertexAttributesFrame ();

}

void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
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
void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
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

  colorbuffer = new_glgrib_opengl_buffer_ptr (geometry->getNumberOfPoints () * sizeof (T));
  T * col = (T *)colorbuffer->map ();
  pack<T>  (data->data (), geometry->getNumberOfPoints (), meta1.valmin, 
            meta1.valmax, meta1.valmis, col);
  col = NULL;
  colorbuffer->unmap ();

  loadHeight <T> (colorbuffer, ld);

  if (opts.mpiview.on)
    setupMpiView (ld, o, slot);

  setupVertexAttributes<T> ();

  if (opts.no_value_pointer.on)
    values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
  else
    values.push_back (data);

  setReady ();
}

void glgrib_field_scalar::setupMpiView (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  int size = geometry->getNumberOfPoints ();

  glgrib_field_metadata mpiview_meta;
  glgrib_field_float_buffer_ptr mpiview;

  ld->load (&mpiview, opts.mpiview.path, opts.geometry, slot, &mpiview_meta, 1, 0);

  float * data = mpiview->data ();

  float pmin = *std::min_element (data, data + size);
  float pmax = *std::max_element (data, data + size);

  int min = (int)pmin;
  int max = (int)pmax;

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

      int j = (int)(*mpiview)[i]-1;

      Disp[j] += lonlat2xyz (glm::vec2 (lon, lat));
      count[j]++;
    }

  for (int i = 0; i < max; i++)
    Disl[i] = xyz2lonlat (glm::normalize (Disp[i] / (float)count[i]));

  if(0)
  for (int i = 0; i < max; i++)
    printf (" %8d | %12.2f, %12.2f\n", i, rad2deg * Disl[i].x, rad2deg * Disl[i].y);

  mpivbuffer = new_glgrib_opengl_buffer_ptr (3 * size * sizeof (float));
  float * mpiv = (float *)mpivbuffer->map ();
  for (int i = 0; i < size; i++)
    {
      int j = (*mpiview)[i]-1;
      mpiv[3*i+0] = (*mpiview)[i];
      mpiv[3*i+1] = Disl[j].x;
      mpiv[3*i+2] = Disl[j].y;
    }
  mpiv = NULL;
  mpivbuffer->unmap ();

}

void glgrib_field_scalar::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  float scale0[3] = {opts.scale, opts.scale, opts.scale};

  if (opts.mpiview.on)
    for (int i = 0; i < 3; i++)
      scale0[i] = scale0[i] / (1.0f + opts.mpiview.scale);

  glgrib_program * program = glgrib_program::load (opts.scalar.points.on 
                                                 ? glgrib_program::SCALAR_POINTS 
                                                 : glgrib_program::GRADIENT_FLAT_SCALE_SCALAR);

  program->use ();





  view.setMVP (program);
  program->setLight (light);
  palette.setRGBA255 (program->programID);
  program->set3fv ("scale0", scale0);
  program->set1f ("valmin", getNormedMinValue ());
  program->set1f ("valmax", getNormedMaxValue ());
  program->set1f ("palmin", palette.getMin ());
  program->set1f ("palmax", palette.getMax ());
  program->set1f ("height_scale", opts.geometry.height.scale);
  program->set1i ("discrete", opts.scalar.discrete.on);
  program->set1f ("mpiview_scale", opts.mpiview.on ? opts.mpiview.scale : 0.0f);

  float missing_color[4] = {(float)opts.scalar.discrete.missing_color.r / 255.0f, 
                            (float)opts.scalar.discrete.missing_color.g / 255.0f, 
                            (float)opts.scalar.discrete.missing_color.b / 255.0f,
                            (float)opts.scalar.discrete.missing_color.a / 255.0f};

  program->set4fv ("RGBAM", missing_color);
 
  geometry->setProgramParameters (program);

  unsigned int Nmax = 1;
  for (int i = 0; i < opts.scalar.pack.bits; i++)
    Nmax = Nmax * 2;

  program->set1i ("Nmax", Nmax-1);
    
  if (opts.scalar.points.on)
    {
      float length = view.pixel_to_dist_at_nadir (10);
    
      program->set1f ("length10", length);
      program->set1f ("pointSiz", opts.scalar.points.size.value);
      program->set1i ("lpointZoo", opts.scalar.points.size.variable.on);
      program->set1i ("factor", opts.scalar.points.size.factor.on);
    
      glBindVertexArray (VertexArrayIDpoints);
    
      unsigned int ind[6] = {0, 1, 2, 2, 3, 0}; 
      glDrawElementsInstanced (GL_TRIANGLES, 6, GL_UNSIGNED_INT, ind, numberOfPoints);
    
      glBindVertexArray (0);
    }
  else
    {
      program->set1i ("smoothed", opts.scalar.smooth.on);
    
      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
      glBindVertexArray (VertexArrayID);

      geometry->renderTriangles ();
      
      glBindVertexArray (0);

      if (opts.scalar.wireframe.on)
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
    }

  view.delMVP (program);

  renderHilo (view);
  
  if (opts.geometry.frame.on)
    renderFrame (view);

}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


