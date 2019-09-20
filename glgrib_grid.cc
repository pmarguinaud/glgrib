#include "glgrib_grid.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

glgrib_grid & glgrib_grid::operator= (const glgrib_grid & other)
{
  clear ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_polygon::operator= (other);
      opts = other.opts;
      setupVertexAttributes ();
      setReady ();
    }
}

void glgrib_grid::setup (const glgrib_options_grid & o)
{
  opts = o;

  numberOfColors = use_alpha () ? 4 : 3;

  std::vector<float> xyz;
  std::vector<unsigned int> ind;

  int ip = 0;

  const int nlatv = 200, nlonv = 2 * opts.resolution;
  const int nlath = opts.resolution, nlonh = 400;

  for (int jlon = 0; jlon < nlonv; jlon++)
    {
      float zlon = 2. * M_PI * (float)jlon / (float)nlonv;
      float coslon = cos (zlon);
      float sinlon = sin (zlon);
      for (int jlat = 0; jlat < nlatv+1; jlat++)
        {   
          float zlat = M_PI / 2. - M_PI * (float)jlat / (float)nlatv;
          float coslat = cos (zlat);
          float sinlat = sin (zlat);
          xyz.push_back (coslon * coslat);
          xyz.push_back (sinlon * coslat);
          xyz.push_back (         sinlat);
          ind.push_back (ip);
          ip++;
        }
      ind.push_back (0xffffffff);
    }
  for (int jlat = 1; jlat < nlath; jlat++)   
    {
      float zlat = M_PI / 2. - M_PI * (float)jlat / (float)nlath;
      float coslat = cos (zlat);
      float sinlat = sin (zlat);
      int nloen = (int)(nlonh * coslat);
      int ip0 = ip;
      for (int jlon = 0; jlon < nloen; jlon++)
        {
          float zlon = 2. * M_PI * (float)jlon / (float)nloen;
          float coslon = cos (zlon);
          float sinlon = sin (zlon);

          xyz.push_back (coslon * coslat);
          xyz.push_back (sinlon * coslat);
          xyz.push_back (         sinlat);
          ind.push_back (ip);
          ip++;
        }
      ind.push_back (ip0);
      ind.push_back (0xffffffff);
    }

  numberOfLines = ind.size ();
  numberOfPoints = xyz.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (xyz.size () * sizeof (float), xyz.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (unsigned int), ind.data ());

  setupVertexAttributes ();

  setReady ();
}

void glgrib_grid::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::MONO);
  program->use ();
  float color[3] = {(float)opts.color.r / 255.0f, 
                    (float)opts.color.g / 255.0f, 
                    (float)opts.color.b / 255.0f};

  view.setMVP (program);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 0);
  program->set1f ("scale", opts.scale);

  glBindVertexArray (VertexArrayID);
  glEnable (GL_PRIMITIVE_RESTART);
  glPrimitiveRestartIndex (0xffffffff);
  glDrawElements (GL_LINE_STRIP, numberOfLines, GL_UNSIGNED_INT, NULL);
  glDisable (GL_PRIMITIVE_RESTART);
  glBindVertexArray (0);

}

