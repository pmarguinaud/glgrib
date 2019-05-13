#include "glgrib_coastlines.h"
#include "glgrib_program.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

glgrib_coastlines & glgrib_coastlines::operator= (const glgrib_coastlines & coastlines)
{
  if ((this != &coastlines) && coastlines.isReady ())
    {
      glgrib_polygon::operator= (coastlines);
      ready_ = false;
      def_from_xyz_col_ind (vertexbuffer, colorbuffer, elementbuffer);
      setReady ();
    }
}


typedef struct head_t
{
  int id;                       /* Unique polygon id number, starting at 0 */
  int n;                        /* Number of points in this polygon */
  int level;                    /* 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake */
  int west, east, south, north; /* min/max extent in micro-degrees */
  int area;                     /* Area of polygon in 1/10 km^2 */
  short int greenwich;          /* Greenwich is 1 if Greenwich is crossed */
  short int source;             /* 0 = CIA WDBII, 1 = WVS */
} head_t;

typedef struct point_t
{
  int x; /* longitude of a point in micro-degrees */
  int y; /* latitude of a point in micro-degrees */
} point_t;

void glgrib_coastlines::init (const glgrib_options_coastlines & opts)
{

  numberOfColors = use_alpha () ? 4 : 3;

  float * xyz = NULL;
  unsigned int * ind = NULL;

  head_t h;
  point_t * pl = NULL;
  FILE * fp = NULL;
  int ip = 0, il = 0;
  numberOfPoints = 0;
  numberOfLines = 0;

  float r = 1.005;
  const float millideg2rad = M_PI / (1000000. * 180.);

  for (int pass = 0; pass < 2; pass++)
    {
      fp = fopen (opts.path.c_str (), "r");
     
      while (1) 
        {   
          fread (&h, sizeof (h), 1, fp);
          pl = (point_t *)realloc (pl, h.n * sizeof (point_t));
          fread (pl, sizeof (point_t), h.n, fp);
          if (h.level == 1)
	    {
	      if (pass == 0)
                {
                  numberOfPoints += h.n;
	          numberOfLines += h.n;
	        }
	      else
                {
                  int ip0 = ip;
                  for (int i = 0; i < h.n; i++)
	            {
                      float coslon = cos (millideg2rad * pl[i].x);
                      float sinlon = sin (millideg2rad * pl[i].x);
                      float coslat = cos (millideg2rad * pl[i].y);
                      float sinlat = sin (millideg2rad * pl[i].y);
                      xyz[ip*3+0] = r * coslon * coslat;
                      xyz[ip*3+1] = r * sinlon * coslat;
                      xyz[ip*3+2] = r *          sinlat;
	              ind[il*2+0] = ip;
	              if (i == h.n - 1)
                        ind[il*2+1] = ip0;
	              else
                        ind[il*2+1] = ip + 1;
	              ip++;
	              il++;
	            }
	        }
            }
          if (feof (fp))
            break;
        }   

      if (pass == 0)
        {
          xyz = (float *)malloc (3 * numberOfPoints * sizeof (float));
          ind = (unsigned int *)malloc (numberOfLines * 2 * sizeof (unsigned int));
	}

      fclose (fp);
    }

  free (pl);

  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (2 * numberOfLines * sizeof (unsigned int), ind);

  def_from_xyz_col_ind (vertexbuffer, colorbuffer, elementbuffer);

  free (ind);
  free (xyz);

  setReady ();
}

void glgrib_coastlines::render (const glgrib_view * view) const
{
  const glgrib_program * program = get_program ();
  GLint color0 = glGetUniformLocation (program->programID, "color0");
  float color[3] = {255., 255., 255.};
  glUniform3fv (color0, 1, color);
  glgrib_polygon::render (view);
}


