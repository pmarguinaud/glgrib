#include "glgrib_test.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


void glgrib_test::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::TEST);
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);

  glEnable (GL_CULL_FACE);
  

  view.delMVP (program);

}

void glgrib_test::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear ();
}

const float rad2deg = 180.0f / M_PI;

void glgrib_test::setup ()
{

  int numberOfPoints;
  glgrib_options_lines opts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  opts.path = "coastlines/shp/GSHHS_c_L1.shp";
  opts.selector = "rowid == 1";
  glgrib_shapelib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &indl, opts.selector);


  for (int i = 0; i < 3; i++)
    printf (" %6d | %8.2f | %8.2f\n", i, rad2deg * lonlat[2*i+0], rad2deg * lonlat[2*i+1]);


  numberOfTriangles = 1;
  numberOfPoints = 3;

  float xyz[numberOfPoints*3] = 
  {
    0., -0.5, -1.,
    0., +1.0, +1.,
    0.,  0.0, +1.
  };

  for (int i = 0; i < 3; i++)
    {
      float coslon = cos (lonlat[2*i+0]), sinlon = sin (lonlat[2*i+0]);
      float coslat = cos (lonlat[2*i+1]), sinlat = sin (lonlat[2*i+1]);
      xyz[3*i+0] = coslon * coslat;
      xyz[3*i+1] = sinlon * coslat;
      xyz[3*i+2] =          sinlat;
      printf (" %8.2f | %8.2f | %8.2f\n", xyz[3*i+0], xyz[3*i+1], xyz[3*i+2]);
    }

  unsigned int ind[3*numberOfTriangles] =
  {
    0, 1, 2
  };

  vertexbuffer = new_glgrib_opengl_buffer_ptr (numberOfPoints * 3 * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (numberOfTriangles * 3 * sizeof (unsigned int), ind);

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

