#include "glgrib_bitmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

static int S4 (unsigned char * h)
{
  return h[0] + 256 * (h[1] + 256 * (h[2] + 256 * h[3]));
}
  
  
void glgrib_bitmap (const std::string & file, unsigned char ** rgb, int * width, int * height)
{
  unsigned char h[54];
  FILE * fp = fopen (file.c_str (), "r");

  if (fread (h, sizeof (h), 1, fp) != 1)
    std::runtime_error (std::string ("Cannot read BMP file : ") + file);
  
  int ioff = S4 (&h[10]); 
  int ncol = S4 (&h[18]);
  int nrow = S4 (&h[22]);

  *rgb = (unsigned char *)malloc (3 * ncol * nrow * sizeof (unsigned char));
  
  fseek (fp, ioff, SEEK_SET);

  if (fread ((*rgb), 3 * ncol * nrow, 1, fp) != 1)
    std::runtime_error (std::string ("Cannot read BMP file : ") + file);

  for (int i = 0; i < ncol * nrow; i++)
    {
      unsigned char r = (*rgb)[3*i+2];
      unsigned char g = (*rgb)[3*i+1];
      unsigned char b = (*rgb)[3*i+0];
      (*rgb)[3*i+0] = r;
      (*rgb)[3*i+1] = g;
      (*rgb)[3*i+2] = b;
    }
  
  fclose (fp);

  *width = ncol;
  *height = nrow;
}
  
