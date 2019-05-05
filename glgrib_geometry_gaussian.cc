#include "glgrib_geometry_gaussian.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glgrib_load.h"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

static const double rad2deg = 180.0 / M_PI;
static const double deg2rad = M_PI / 180.0;

#define MODULO(x, y) ((x)%(y))
#define JDLON(JLON1, JLON2) (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1))
#define JNEXT(JLON, ILOEN) (1 + MODULO ((JLON), (ILOEN)))


#define PRINT(a,b,c) \
  do {                                                            \
      *(inds++) = (a)-1; *(inds++) = (b)-1; *(inds++) = (c)-1;    \
  } while (0)

static 
void glgauss (const long int Nj, const long int pl[], unsigned int * ind, const int nstripe, int indcnt[])
{
  int iglooff[Nj];
  int indcntoff[nstripe];
  
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj-1; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  indcntoff[0] = 0;
  for (int istripe = 1; istripe < nstripe; istripe++)
    indcntoff[istripe] = indcntoff[istripe-1] + indcnt[istripe-1];

#pragma omp parallel for 
  for (int istripe = 0; istripe < nstripe; istripe++)
    {
      int jlat1 = 1 + ((istripe + 0) * (Nj-1)) / nstripe;
      int jlat2 = 0 + ((istripe + 1) * (Nj-1)) / nstripe;
      unsigned int * inds = ind + 3 * indcntoff[istripe];

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

}




void glgrib_geometry_gaussian::gencoords (float * px, float * py) const
{
  for (int jglo = 0, jlat = 1; jlat <= Nj; jlat++)
    {
      float coordy = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
        {
          float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
          px[jglo] = rad2deg * coordx;
          py[jglo] = rad2deg * coordy;
        }
    }
}

void glgrib_geometry_gaussian::genlatlon (float * plat, float * plon) const
{
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

          float X = coslon * coslat;
          float Y = sinlon * coslat;
          float Z =          sinlat;

          glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
          XYZ = rot * XYZ;

          plon[jglo] = rad2deg * atan2 (XYZ.y, XYZ.x);
          plat[jglo] = rad2deg * asin (XYZ.z);

        }
    }
}

int glgrib_geometry_gaussian::size () const
{
  return jglooff[Nj-1] + pl[Nj-1];
}

glgrib_geometry_gaussian::glgrib_geometry_gaussian (const glgrib_options & opts, codes_handle * h)
{
  float * xyz = NULL;
  unsigned int * ind = NULL;
  const int nstripe = 8;
  int indoff[nstripe];


  size_t v_len = 0;
  codes_get_size (h, "values", &v_len);
      
  double vmin, vmax, vmis;

  double * v = NULL;
  if (opts.landscape.orography > 0.0f)
    {
      v = (double *)malloc (v_len * sizeof (double));
      codes_get_double_array (h, "values", v, &v_len);
      codes_get_double (h, "maximum",      &vmax);
      codes_get_double (h, "minimum",      &vmin);
      codes_get_double (h, "missingValue", &vmis);
    }

  if (codes_is_defined (h, "stretchingFactor"))
    codes_get_double (h, "stretchingFactor", &stretchingFactor);
  if (codes_is_defined (h, "latitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "latitudeOfStretchingPoleInDegrees",
                      &latitudeOfStretchingPoleInDegrees);
  if (codes_is_defined (h, "longitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "longitudeOfStretchingPoleInDegrees", 
                      &longitudeOfStretchingPoleInDegrees);
  
  bool do_rot_str = true;

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
  
  if (do_rot_str) 
  if (latitudeOfStretchingPoleInDegrees != 90.0f 
   && (latitudeOfStretchingPoleInDegrees != 0.0f 
    && longitudeOfStretchingPoleInDegrees != 0.0f))
    {
      rot = glm::rotate (glm::mat4 (1.0f),
                         glm::radians (90.0f-(float)latitudeOfStretchingPoleInDegrees), 
                         glm::vec3 (-sinf (glm::radians (longitudeOfStretchingPoleInDegrees)),
                                    +cosf (glm::radians (longitudeOfStretchingPoleInDegrees)),
                                    0.0f)) 
          *
            glm::rotate (glm::mat4 (1.0f),
                         glm::radians (180.0f+(float)longitudeOfStretchingPoleInDegrees),
                         glm::vec3 (0.0f, 0.0f, 1.0f));
    }

  size_t pl_len;
  codes_get_long (h, "Nj", &Nj);
  codes_get_size (h, "pl", &pl_len);
  pl = (long int *)malloc (sizeof (long int) * pl_len);
  codes_get_long_array (h, "pl", pl, &pl_len);

  // Compute number of triangles
  nt = 0;
  for (int jlat = 1; jlat < Nj; jlat++)
    nt += pl[jlat-1] + pl[jlat];
  
  // Compute number of triangles per stripe
  for (int istripe = 0; istripe < nstripe; istripe++)
    {
      int jlat1 = 1 + ((istripe + 0) * (Nj-1)) / nstripe;
      int jlat2 = 0 + ((istripe + 1) * (Nj-1)) / nstripe;
      indoff[istripe] = 0;
      for (int jlat = jlat1; jlat <= jlat2; jlat++)
        indoff[istripe] += pl[jlat-1] + pl[jlat];
    }

  ind = (unsigned int *)malloc (3 * nt * sizeof (unsigned int));
  // OpenMP generation of triangles
  glgauss (Nj, pl, ind, nstripe, indoff);
      
  xyz = (float *)malloc (3 * sizeof (float) * v_len);
  np  = v_len;

  int iglooff[Nj];
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  // OpenMP generation of coordinates
#pragma omp parallel for 
  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      float coordy = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
      float sincoordy = sin (coordy);
      float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
      float coslat = cos (lat); float sinlat = sin (lat);
      int jglo = iglooff[jlat-1];
      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
        {
          float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
          float lon = coordx;
          float coslon = cos (lon); float sinlon = sin (lon);

          float radius;
 
          if (opts.landscape.orography > 0.0f)
            radius = (1.0 + ((v[jglo] == vmis) ? 0. : 0.05 * v[jglo]/vmax));
          else
            radius = 1.0f;

          float X = coslon * coslat * radius;
          float Y = sinlon * coslat * radius;
          float Z =          sinlat * radius;

          glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
          XYZ = rot * XYZ;

          xyz[3*jglo+0] = XYZ.x;
          xyz[3*jglo+1] = XYZ.y;
          xyz[3*jglo+2] = XYZ.z;

        }
    }

  if (opts.landscape.orography > 0.0f)
    free (v);

  glBindVertexArray (0);
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);

  free (xyz); xyz = NULL;
  free (ind); ind = NULL;
  
  jglooff = (int *)malloc (Nj * sizeof (int));

  jglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    jglooff[jlat-1] = jglooff[jlat-2] + pl[jlat-2];
}

glgrib_geometry_gaussian::~glgrib_geometry_gaussian ()
{
  if (pl)
    free (pl);
  pl = NULL;
  if (jglooff)
    free (jglooff);
  jglooff = NULL;
}

int glgrib_geometry_gaussian::latlon2index (float lat, float lon) const
{
  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float coslat = cos (lat), sinlat = sin (lat);
  float coslon = cos (lon), sinlon = sin (lon);
  float x = coslon * coslat;
  float y = sinlon * coslat;
  float z =          sinlat;

  glm::vec4 xyz = glm::vec4 (x, y, z, 0.0f);
  glm::vec4 XYZ = glm::inverse (rot) * xyz;

  float X = XYZ.x;
  float Y = XYZ.y;
  float Z = XYZ.z;

  lon = atan2 (Y, X); 
  lat = asin (Z);

  float coordx = lon;
  sinlat = sin (lat);
  float coordy = asin ((-omc2 + sinlat * opc2) / (opc2 - sinlat * omc2));

  int jlat = round ((0.5 - coordy / M_PI) * (Nj + 1));
  if (jlat > Nj)
    jlat = Nj;
  int jlon = round (pl[jlat-1] * (coordx / (2. * M_PI)));
  if (jlon < 0)
    jlon += pl[jlat-1];


  return jglooff[jlat-1] + jlon;
}


