#include "glgrib_grid.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glgrib_grid & glgrib_grid::operator= (const glgrib_grid & other)
{
  cleanup ();
  if ((this != &other) && other.isReady ())
    {
      glgrib_polygon::operator= (other);
      opts = other.opts;
      setupVertexAttributes ();
      setReady ();
    }
}

void glgrib_grid::init (const glgrib_options_grid & o)
{
  opts = o;

  numberOfColors = use_alpha () ? 4 : 3;

  float * xyz = NULL;
  unsigned int * ind = NULL;

  int ip = 0, il = 0;
  numberOfPoints = 0;
  numberOfLines = 0;

  const int nlatv = 200, nlonv = 2 * opts.resolution;
  const int nlath = opts.resolution, nlonh = 400;

  for (int pass = 0; pass < 2; pass++)
    {

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
              if (pass == 1)
                {
                  xyz[ip*3+0] = coslon * coslat;
                  xyz[ip*3+1] = sinlon * coslat;
                  xyz[ip*3+2] =          sinlat;
                  if (jlat < nlatv)
                    {
                      ind[il*2+0] = ip;
                      ind[il*2+1] = ip + 1;
                      il++;
                    }
                  ip++;
                }
              else
                {
                  numberOfPoints++;
                  if (jlat < nlatv)
                    numberOfLines++;
                }
            }
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

              if (pass == 1) 
                {
                  xyz[ip*3+0] = coslon * coslat;
                  xyz[ip*3+1] = sinlon * coslat;
                  xyz[ip*3+2] =          sinlat;
                  ind[il*2+0] = ip;
                  if (jlon < nloen-1)
                    ind[il*2+1] = ip + 1;
                  else
                    ind[il*2+1] = ip0;
                  ip++;
                  il++;
                }
              else
                {
                  numberOfPoints++;
                  numberOfLines++;
                }

            }
        }

      if (pass == 0)
        {
          xyz = (float *)malloc (3 * numberOfPoints * sizeof (float));
          ind = (unsigned int *)malloc (numberOfLines * 2 * sizeof (unsigned int));
	}

    }

  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (2 * numberOfLines * sizeof (unsigned int), ind);

  setupVertexAttributes ();

  free (ind);
  free (xyz);

  setReady ();
}

void glgrib_grid::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::MONO);
  program->use ();
  float color[3] = {(float)opts.color.r, (float)opts.color.g, (float)opts.color.b};

  view.setMVP (program);
  program->set3fv ("color0", color);
  program->set1i ("do_alpha", 0);
  glgrib_polygon::render (view, light);
}

