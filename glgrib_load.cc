#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>

#include "glgrib_load.h"

#define MODULO(x, y) ((x)%(y))
#define JDLON(JLON1, JLON2) (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1))
#define JNEXT(JLON, ILOEN) (1 + MODULO ((JLON), (ILOEN)))


#define PRINT(a,b,c) \
  do {                                                                                          \
    if (pass == 1)                                                                              \
      {                                                                                         \
        (*ind)++;                                                                               \
      }                                                                                         \
    else if (pass == 2)                                                                         \
      {                                                                                         \
        *(ind++) = (a)-1; *(ind++) = (b)-1; *(ind++) = (c)-1;                                   \
      }                                                                                         \
  } while (0)

static 
void glgauss (const long int Nj, const long int pl[], int pass, unsigned int * ind)
{
  int jglooff = 0;

  if (pass == 1)
    *ind = 0;

  for (int jlat = 1; jlat <= Nj-1; jlat++)
    {
      int iloen1 = pl[jlat - 1];
      int iloen2 = pl[jlat + 0];
      int jglooff1 = jglooff + 0;
      int jglooff2 = jglooff + iloen1;


      if (iloen1 == iloen2) 
        {
          for (int jlon1 = 1; jlon1 <= iloen1; jlon1++)
            {
              int jlon2 = jlon1;
              int ica = jglooff1 + jlon1;
              int icb = jglooff2 + jlon2;
              int icc = jglooff2 + JNEXT (jlon2, iloen2);
              int icd = jglooff1 + JNEXT (jlon1, iloen1);
	      PRINT (ica, icb, icc);
	      PRINT (icc, icd, ica);
            }
        }
      else 
        {
          int jlon1 = 1;
          int jlon2 = 1;
          for (;;)
            {
              int ica = 0, icb = 0, icc = 0;
  
              int idlonc = JDLON (jlon1, jlon2);
              int idlonn = JDLON (JNEXT (jlon1, iloen1), JNEXT (jlon2, iloen2));
         
              if (idlonc > 0 || ((idlonc == 0) && idlonn > 0)) 
                {
                  int jlon = JNEXT (jlon2, iloen2);
                  ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon;
                  jlon2 = jlon;
                }
              else if (idlonc < 0 || ((idlonc == 0) && idlonn < 0)) 
                {
                  int jlon = JNEXT (jlon1, iloen1);
                  ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon;
                  jlon1 = jlon;
                }
              else
                {
                  abort ();
                }
         
              PRINT (ica, icb, icc);
             
              if ((jlon1 == 1) && (jlon2 == iloen2)) 
                {
                  int jlon = JNEXT (jlon2, iloen2);
                  ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon;
                  PRINT (ica, icb, icc);
                }
              else if ((jlon1 == iloen1) && (jlon2 == 1)) 
                {
                  int jlon = JNEXT (jlon1, iloen1);
                  ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon;
                  PRINT (ica, icb, icc);
                }
              else
                {
                  continue;
                }
              break;
          }
     
     
        }

      jglooff = jglooff + pl [jlat-1];
    }


}


void glgrib_load_z (const char * geom, int * np, float ** xyz, 
                    unsigned int * nt, unsigned int ** ind)
{
  FILE * in = NULL;
  long int * pl = NULL;
  long int Nj;
  char file[64];

  sprintf (file, "Z_%s.grb", geom);
  
  in = fopen (file, "r");

  *xyz = NULL;
  *ind = NULL;

  int err = 0;
  size_t v_len = 0;
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
  codes_get_size (h, "values", &v_len);
      
  double vmin, vmax, vmis;
  double * v = (double *)malloc (v_len * sizeof (double));

  codes_get_double_array (h, "values", v, &v_len);
  codes_get_double (h, "maximum",      &vmax);
  codes_get_double (h, "minimum",      &vmin);
  codes_get_double (h, "missingValue", &vmis);
      
  size_t pl_len;
  codes_get_long (h, "Nj", &Nj);
  codes_get_size (h, "pl", &pl_len);
  pl = (long int *)malloc (sizeof (long int) * pl_len);
  codes_get_long_array (h, "pl", pl, &pl_len);
  glgauss (Nj, pl, 1, nt);
  *ind = (unsigned int *)malloc (3 * (*nt) * sizeof (unsigned int));
  glgauss (Nj, pl, 2, *ind);
  codes_handle_delete (h);
      
      
  *xyz = (float *)malloc (3 * sizeof (float) * v_len);
  *np  = v_len;
  for (int jglo = 0, jlat = 1; jlat <= Nj; jlat++)
    {
      float lat = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
      float coslat = cos (lat); float sinlat = sin (lat);
      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
        {
          float lon = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
          float coslon = cos (lon); float sinlon = sin (lon);
          float radius = (1.0 + ((v[jglo] == vmis) ? 0. : 0.05 * v[jglo]/vmax));
          (*xyz)[3*jglo+0] = coslon * coslat * radius;
          (*xyz)[3*jglo+1] = sinlon * coslat * radius;
          (*xyz)[3*jglo+2] =          sinlat * radius;
        }
    }
  free (v);

  fclose (in);
  free (pl);
  
}

void glgrib_load_rgb (const char * geom, unsigned char ** col, int use_alpha)
{
  FILE * in = NULL;
  int ncol = use_alpha ? 4 : 3;
  char file[64];

  sprintf (file, "RGB_%s.grb", geom);
  
  in = fopen (file, "r");

  *col = NULL;

  for (int ifld = 0; ifld < 3; ifld++)
    {
      int err = 0;
      size_t v_len = 0;
      codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
      codes_get_size (h, "values", &v_len);
      
      double vmin, vmax, vmis;
      double * v = (double *)malloc (v_len * sizeof (double));

      codes_get_double_array (h, "values", v, &v_len);
      codes_get_double (h, "maximum",      &vmax);
      codes_get_double (h, "minimum",      &vmin);
      codes_get_double (h, "missingValue", &vmis);
      codes_handle_delete (h);
      
      if (*col == NULL)
        {
          *col = (unsigned char *)malloc (ncol * sizeof (unsigned char) * v_len);
          if (use_alpha)
            for (int jglo = 0; jglo < v_len; jglo++)
              (*col)[ncol*jglo+3] = 255;
	}

      for (int jglo = 0; jglo < v_len; jglo++)
        (*col)[ncol*jglo+ifld] = 255 * v[jglo];
      
      free (v);
    }

  fclose (in);
}

