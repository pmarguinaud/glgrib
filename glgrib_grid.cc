#include "glgrib_grid.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

void glgrib_grid::init (const glgrib_options & opts)
{

  ncol = use_alpha () ? 4 : 3;

  float * xyz = NULL;
  unsigned int * ind = NULL;

  int ip = 0, il = 0;
  np = 0;
  nl = 0;

  float r = 1.005;

  const int nlatv = 200, nlonv = 2 * opts.grid.resolution;
  const int nlath = opts.grid.resolution, nlonh = 400;

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
                  xyz[ip*3+0] = r * coslon * coslat;
                  xyz[ip*3+1] = r * sinlon * coslat;
                  xyz[ip*3+2] = r *          sinlat;
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
                  np++;
                  if (jlat < nlatv)
                    nl++;
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
                  xyz[ip*3+0] = r * coslon * coslat;
                  xyz[ip*3+1] = r * sinlon * coslat;
                  xyz[ip*3+2] = r *          sinlat;
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
                  np++;
                  nl++;
                }

            }
        }

      if (pass == 0)
        {
          xyz = (float *)malloc (3 * np * sizeof (float));
          ind = (unsigned int *)malloc (nl * 2 * sizeof (unsigned int));
	}

    }


  def_from_xyz_col_ind (xyz, NULL, ind);

  free (ind);
  free (xyz);

  setReady ();
}

void glgrib_grid::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program ();
  GLint color0 = glGetUniformLocation (program->programID, "color0");
  float color[3] = {255., 255., 255.};
  glUniform3fv (color0, 1, color);
  glgrib_polygon::render (view);
}

