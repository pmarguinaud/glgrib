#include <stdio.h>
#include <stdlib.h>
#include <eccodes.h>

#include "glgrib_load.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


#define MODULO(x, y) ((x)%(y))
#define JDLON(JLON1, JLON2) (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1))
#define JNEXT(JLON, ILOEN) (1 + MODULO ((JLON), (ILOEN)))


#define PRINT(a,b,c) \
  do {                                                                                          \
    if (pass == 1)                                                                              \
      {                                                                                         \
        indcnt[istripe]++;                                                                      \
      }                                                                                         \
    else if (pass == 2)                                                                         \
      {                                                                                         \
        *(inds++) = (a)-1; *(inds++) = (b)-1; *(inds++) = (c)-1;                                \
      }                                                                                         \
  } while (0)

static 
void glgauss (const long int Nj, const long int pl[], int pass, unsigned int * ind, const int nstripe, int indcnt[])
{
  int iglooff[Nj];
  int indcntoff[nstripe];
  
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj-1; jlat++)
     iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  if (pass == 1)
    *ind = 0;

  if (pass == 2)
    {
      indcntoff[0] = 0;
      for (int istripe = 1; istripe < nstripe; istripe++)
        indcntoff[istripe] = indcntoff[istripe-1] + indcnt[istripe-1];
    }

#pragma omp parallel for 
  for (int istripe = 0; istripe < nstripe; istripe++)
    {
      int jlat1 = 1 + ((istripe + 0) * (Nj-1)) / nstripe;
      int jlat2 = 0 + ((istripe + 1) * (Nj-1)) / nstripe;
      unsigned int * inds = ind + 3 * indcntoff[istripe];

      if (pass == 1)
        indcnt[istripe] = 0;

      for (int jlat = jlat1; jlat <= jlat2; jlat++)
        {
          int iloen1 = pl[jlat - 1];
          int iloen2 = pl[jlat + 0];
          int jglooff1 = iglooff[jlat-1] + 0;
          int jglooff2 = iglooff[jlat-1] + iloen1;
     
     
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
                  int jlon1n = JNEXT (jlon1, iloen1);
                  int jlon2n = JNEXT (jlon2, iloen2);
                  int idlonn = JDLON (jlon1n, jlon2n);

#define AV1 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;  \
    jlon1 = jlon1n;                                                           \
  } while (0)

#define AV2 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;  \
    jlon2 = jlon2n;                                                           \
  } while (0)

                  if (idlonc > 0 || ((idlonc == 0) && (idlonn > 0)))
                    {
                      if (jlon2n != 1)
                        AV2;
                      else
                        AV1;
                    }
                  else if (idlonc < 0 || ((idlonc == 0) && (idlonn < 0))) 
                    {
                      if (jlon1n != 1)
                        AV1;
                      else
                        AV2;
                    }
                  else
                    {
                      abort ();
                    }
             
                  PRINT (ica, icb, icc);
                 
                  if ((jlon1 == 1) && (jlon2 == iloen2)) 
                    {
                      ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;
                      PRINT (ica, icb, icc);
                    }
                  else if ((jlon1 == iloen1) && (jlon2 == 1)) 
                    {
                      ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;
                      PRINT (ica, icb, icc);
                    }
                  else
                    {
                      continue;
                    }
                  break;
              }
         
         
            }

        }

    }

  if (pass == 1)
    for (int istripe = 0; istripe < nstripe; istripe++)
      *ind += indcnt[istripe];

}




void glgrib_load_z (const char * geom, int * np, float ** xyz, 
                    unsigned int * nt, unsigned int ** ind)
{
  FILE * in = NULL;
  long int * pl = NULL;
  long int Nj;
  char file[64];
  const int nstripe = 8;
  int indoff[nstripe];

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

  double stretchingFactor = 1.0f;
  if (codes_is_defined (h, "stretchingFactor"))
    codes_get_double (h, "stretchingFactor", &stretchingFactor);
  double latitudeOfStretchingPoleInDegrees = 90.0f;
  if (codes_is_defined (h, "latitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "latitudeOfStretchingPoleInDegrees",
                      &latitudeOfStretchingPoleInDegrees);
  double longitudeOfStretchingPoleInDegrees = 0.0f;
  if (codes_is_defined (h, "longitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "longitudeOfStretchingPoleInDegrees", 
                      &longitudeOfStretchingPoleInDegrees);
  

  bool do_rot_str = true;

  float omc2;
  float opc2;
  if (! do_rot_str)
    {
      omc2 = 0.0f;
      opc2 = 2.0f;
    }
  else
    {
      omc2 = 1.0f - 1.0f / (stretchingFactor * stretchingFactor);
      opc2 = 1.0f + 1.0f / (stretchingFactor * stretchingFactor);
    }
  
  glm::mat4 rot = glm::mat4 (1.0f);

  if (do_rot_str) 
  if (latitudeOfStretchingPoleInDegrees != 90.0f 
   && (latitudeOfStretchingPoleInDegrees != 0.0f 
    && longitudeOfStretchingPoleInDegrees != 0.0f))
    {
      rot = 
	      glm::rotate (glm::mat4 (1.0f),
                         glm::radians (90.0f-(float)latitudeOfStretchingPoleInDegrees), 
                         glm::vec3 (-sinf (glm::radians (longitudeOfStretchingPoleInDegrees)), 
                                    +cosf (glm::radians (longitudeOfStretchingPoleInDegrees)),
                                    0.0f)); 
    }

  size_t pl_len;
  codes_get_long (h, "Nj", &Nj);
  codes_get_size (h, "pl", &pl_len);
  pl = (long int *)malloc (sizeof (long int) * pl_len);
  codes_get_long_array (h, "pl", pl, &pl_len);
  glgauss (Nj, pl, 1, nt, nstripe, indoff);
  *ind = (unsigned int *)malloc (3 * (*nt) * sizeof (unsigned int));
  glgauss (Nj, pl, 2, *ind, nstripe, indoff);
  codes_handle_delete (h);
      
  *xyz = (float *)malloc (3 * sizeof (float) * v_len);
  *np  = v_len;
  for (int jglo = 0, jlat = 1; jlat <= Nj; jlat++)
    {
      float coordy = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
      float sincoordy = sin (coordy);
      float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
      float coslat = cos (lat); float sinlat = sin (lat);
      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
        {
          float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
          float lon = coordx;
          float coslon = cos (lon); float sinlon = sin (lon);
          float radius = (1.0 + ((v[jglo] == vmis) ? 0. : 0.05 * v[jglo]/vmax));

          float X = coslon * coslat * radius;
          float Y = sinlon * coslat * radius;
          float Z =          sinlat * radius;

          glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
          XYZ = rot * XYZ;

          (*xyz)[3*jglo+0] = XYZ.x;
          (*xyz)[3*jglo+1] = XYZ.y;
          (*xyz)[3*jglo+2] = XYZ.z;

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

void glgrib_load (const char * geom, float ** val, int what)
{
  FILE * in = NULL;
  char file[64];

  sprintf (file, "N_%s.grb", geom);
  
  in = fopen (file, "r");

  *val = NULL;

  int err = 0;
  codes_handle * h = codes_handle_new_from_file (0, in, PRODUCT_GRIB, &err);
  size_t pl_len;
  size_t v_len = 0;
  codes_get_size (h, "pl", &pl_len);
  codes_get_size (h, "values", &v_len);
  long int * pl = (long int *)malloc (sizeof (long int) * pl_len);
  codes_get_long_array (h, "pl", pl, &pl_len);
  

  double vmin, vmax;
  double * v = NULL;
  codes_get_double (h, "maximum", &vmax);
  codes_get_double (h, "minimum", &vmin);
  if (what == 2)
    {
      v = (double *)malloc (sizeof (double) * v_len);
      codes_get_double_array (h, "values", v, &v_len);
    }

  codes_handle_delete (h);

  *val = (float *)malloc (sizeof (float) * v_len);

  for (int jlat = 1, jglo = 0; jlat <= pl_len; jlat++)
    for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
      switch (what)
        {
          case 0: (*val)[jglo] = ((int)(10 * (float)(jlat-1)/(float)pl_len)) / 10.0f; break;
          case 1: (*val)[jglo] = ((int)(10 * (float)(jlon-1)/(float)pl[jlat-1])) / 10.0f; break;
          case 2: (*val)[jglo] = ((int)(10 * (v[jglo] - vmin) / (vmax - vmin))) / 10.0f; break;
        }

  
  free (pl);

  if (v != NULL)
    free (v);

  fclose (in);
}

