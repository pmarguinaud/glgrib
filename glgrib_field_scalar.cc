#include "glgrib_field_scalar.h"
#include "glgrib_program.h"
#include "glgrib_palette.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

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
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  // Colored field
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->bindCoordinates (0);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 

  geometry->bindTriangles ();

  bindHeight (2);

  glBindVertexArray (0); 


  // Points
  glGenVertexArrays (1, &VertexArrayIDpoints);
  glBindVertexArray (VertexArrayIDpoints);

  geometry->bindCoordinates (0);
  glVertexAttribDivisor (0, 1);
  
  colorbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, numberOfColors, GL_UNSIGNED_BYTE, GL_TRUE, numberOfColors * sizeof (unsigned char), NULL); 
  glVertexAttribDivisor (1, 1);

  if (heightbuffer)
    {
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (2);
      glVertexAttribPointer (2, 1, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
      glVertexAttribDivisor (2, 1);
    }
  else
    {
      glDisableVertexAttribArray (2);
      glVertexAttrib1f (2, 0.0f);
    }

  glBindVertexArray (0); 
}

static
void walk (const float * tval, int it0, const_glgrib_geometry_ptr geometry)
{

  int jglo0[3], itri0[3]; glm::vec3 xyz0[3];
  geometry->getTriangleNeighbours (it0, jglo0, itri0, xyz0);


  int it1 = -1;
  int it2 = itri0[0];

  auto adj = [jglo0] (int jglo2)
  {
    for (int i = 0; i < 3; i++)
      if (jglo0[i] == jglo2)
        return true;
    return false;
  };

  std::cout << " it0 = " << it0 << std::endl;
  std::cout << " itri0 = " << itri0[0] << " " << itri0[1] << " " << itri0[2] << std::endl;
  while (1)
    {
      if (it2 == -1)
        break;

      std::cout << " it2 = " << it2 << std::endl;

      int jglo2[3], itri2[3]; glm::vec3 xyz2[3];
      geometry->getTriangleNeighbours (it2, jglo2, itri2, xyz2);

      std::cout << " itri2 = " << itri2[0] << " " << itri2[1] << " " << itri2[2] << std::endl;
      std::cout << " jglo2 = " << jglo2[0] << " " << jglo2[1] << " " << jglo2[2] << std::endl;

      for (int i = 0; i < 3; i++)
        {
          int j = (i + 1) % 3;
          int k = (i + 2) % 3;
          int it3 = itri2[i];

	  if (it3 == -1)
            continue;
	  if (it3 == it1)
            continue;
	  if (it3 == it0)
            continue;
          if (adj (jglo2[i]) || adj (jglo2[j]))
            {
              it1 = it2;
              it2 = it3;
	      goto found;
	    }
         }

      break;
found:
      continue;
      if (it2 == itri0[0])
        break;
    }

  std::cout << std::endl;

  


}

static
void hilo (const_glgrib_geometry_ptr geometry, glgrib_field_float_buffer_ptr data)
{
  const int nt = geometry->getNumberOfTriangles ();
  float * tval = new float[nt];
  const float * vval = data->data ();

#pragma omp parallel for
  for (int it = 0; it < nt; it++)
    {
      int jglo[3];
      geometry->getTriangleVertices (it, jglo);
      tval[it] = (vval[jglo[0]] 
                + vval[jglo[1]] 
                + vval[jglo[2]]) / 3.0f;
    }

  for (int it = 0; it < 1; it++)
    walk (tval, it, geometry);

#ifdef UNDEF
  const int N = 16;

  int nlo = 0, nhi = 0;
//#pragma omp parallel for
  for (int j = 0; j < N; j++)
    {
      int it0 = (nt * (j + 0)) / N;
      int it1 = (nt * (j + 1)) / N;
      for (int it = it0; it < it1; it++)
        {
          int jglo[3], itri[3]; glm::vec3 xyz[3];
          geometry->getTriangleNeighbours (it, jglo, itri, xyz);
          bool hi = true, lo = true;
          for (int i = 0; i < 3; i++)
            {
              if (itri[i] < 0)
                continue;
              hi = hi && (tval[itri[i]] < tval[it]);
              lo = lo && (tval[itri[i]] > tval[it]);
            }
          if (hi)
            nhi++;
          if (lo)
            nlo++;
          if (lo || hi)
            {
              const float rad2deg = 1.0f; //180.0f / M_PI;
              glm::vec3 pos = glm::normalize ((xyz[0] + xyz[1] + xyz[2]) / 3.0f);
              glm::vec2 latlon = geometry->conformal2latlon (geometry->xyz2conformal (pos));

              if (lo) std::cout << " lo = " << rad2deg * latlon.x << " " << rad2deg * latlon.y << std::endl;
              if (hi) std::cout << " hi = " << rad2deg * latlon.x << " " << rad2deg * latlon.y << std::endl;
 
            }
        }
    }
  std::cout << " np  = " << geometry->getNumberOfPoints () << std::endl;
  std::cout << " nhi = " << nhi << std::endl;
  std::cout << " nlo = " << nlo << std::endl;
#endif


  delete [] tval;
}



void glgrib_field_scalar::setup (glgrib_loader * ld, const glgrib_options_field & o, float slot)
{
  opts = o;


  glgrib_field_metadata meta1;

  glgrib_field_float_buffer_ptr data;
  ld->load (&data, opts.path, opts.geometry, slot, &meta1, 1, 0, opts.diff.on);
  meta.push_back (meta1);

  palette = glgrib_palette::create (opts.palette, getNormedMinValue (), getNormedMaxValue ());

  geometry = glgrib_geometry::load (ld, opts.path[0], opts.geometry);

  hilo (geometry, data);

  numberOfColors = 1;

  colorbuffer = new_glgrib_opengl_buffer_ptr (numberOfColors * geometry->getNumberOfPoints () * sizeof (unsigned char));
  unsigned char * col = (unsigned char *)colorbuffer->map ();

  for (int i = 0; i < geometry->getNumberOfPoints (); i++)
    if ((*data)[i] == meta1.valmis)
      col[i] = 0;
    else
      col[i] = 1 + (int)(254 * ((*data)[i] - meta1.valmin)/(meta1.valmax - meta1.valmin));

  col = NULL;
  colorbuffer->unmap ();

  loadHeight (colorbuffer, ld);

  setupVertexAttributes ();

  if (opts.no_value_pointer.on)
    values.push_back (new_glgrib_field_float_buffer_ptr ((float*)NULL));
  else
    values.push_back (data);

  setReady ();
}

void glgrib_field_scalar::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  float scale0[3] = {opts.scale, opts.scale, opts.scale};

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

}

glgrib_field_scalar::~glgrib_field_scalar ()
{
}


