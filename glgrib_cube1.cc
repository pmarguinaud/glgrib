#include "glgrib_cube1.h"

#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


void cube1_t::init ()
{
  ncol = use_alpha ? 4 : 3;
  nt = 12;
  np = 8;

  float * xyz = (float *)malloc (3 * np * sizeof (float));
  unsigned char * col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
  unsigned int * ind = (unsigned int *)malloc (nt * 3 * sizeof (unsigned int));

  float s = 0.8;

  xyz[0*3+0] = -s; xyz[0*3+1] = -s; xyz[0*3+2] = -s;
  xyz[1*3+0] = -s; xyz[1*3+1] = +s; xyz[1*3+2] = -s;
  xyz[2*3+0] = -s; xyz[2*3+1] = +s; xyz[2*3+2] = +s;
  xyz[3*3+0] = -s; xyz[3*3+1] = -s; xyz[3*3+2] = +s;
  xyz[4*3+0] = +s; xyz[4*3+1] = -s; xyz[4*3+2] = -s;
  xyz[5*3+0] = +s; xyz[5*3+1] = +s; xyz[5*3+2] = -s;
  xyz[6*3+0] = +s; xyz[6*3+1] = +s; xyz[6*3+2] = +s;
  xyz[7*3+0] = +s; xyz[7*3+1] = -s; xyz[7*3+2] = +s;


  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 255;

  for (int i = 0; i < np; i++)
  for (int j = 0; j < 3; j++)
    col[ncol*i+j] = 255 * ((s + xyz[i*3+j]) / (2 * s));

  ind[ 0] = 4; ind[ 1] = 5; ind[ 2] = 6;
  ind[ 3] = 4; ind[ 4] = 6; ind[ 5] = 7;

  ind[ 6] = 1; ind[ 7] = 0; ind[ 8] = 2;
  ind[ 9] = 2; ind[10] = 0; ind[11] = 3;

  ind[12] = 1; ind[13] = 2; ind[14] = 6;
  ind[15] = 1; ind[16] = 6; ind[17] = 5;

  ind[18] = 2; ind[19] = 3; ind[20] = 6;
  ind[21] = 3; ind[22] = 7; ind[23] = 6;

  ind[24] = 3; ind[25] = 0; ind[26] = 4;
  ind[27] = 3; ind[28] = 4; ind[29] = 7;

  ind[30] = 5; ind[31] = 4; ind[32] = 1;
  ind[33] = 1; ind[34] = 4; ind[36] = 0;

  
  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}
