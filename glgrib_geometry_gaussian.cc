#include "glgrib_geometry_gaussian.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>
#include <openssl/md5.h>

#include <iostream>
#include <limits>
#include <algorithm>
#include <exception>

static
void compute_latgauss (int kn, double * latgauss)
{
  const double pi = M_PI;
  const int itemax = 20;
  const double zeps = std::numeric_limits <double>::epsilon ();
  
  double * zfn    = new double[(kn+1)*(kn+1)]; 
  double * zfnlat = new double[kn/2+2]; 
  double * zmod   = new double[kn]; 

  //     ------------------------------------------------------------------
  //        1. initialization.
  //           ---------------
  // 
  //        1.1 calculation of zfnlat.
  //            (fourier coefficients of series expansion for
  //            the ordinary legendre polynomials).
  // belousov, swarztrauber use zfn(0,0)=sqrt(2._jprb)
  // ifs normalisation chosen to be 0.5*integral(pnm**2) = 1 

  zfn[0] = 2.;
#pragma omp parallel for
  for (int jn = 1; jn <= kn; jn++)
    {
      double zfnn = zfn[0];
      for (int jgl = 1; jgl <= jn; jgl++)
        zfnn = zfnn * sqrt (1. - 0.25 / (jgl * jgl));
      int iodd = jn % 2;
      zfn[jn*(kn+1)+jn] = zfnn;
      for (int jgl = 2; jgl <= jn-iodd; jgl += 2)
        zfn[jn*(kn+1)+jn-jgl] = zfn[jn*(kn+1)+jn-jgl+2] 
         * (double)((jgl-1)*(2*jn-jgl+2)) / (double)(jgl*(2*jn-jgl+1));
    }
  
  int iodd = kn % 2;
  int ins2 = kn / 2 + kn % 2;

#pragma omp parallel for
  for (int jgl = iodd; jgl <= kn; jgl += 2)
    {
      int ik = iodd + (jgl - iodd) / 2;
      zfnlat[ik] = zfn[kn*(kn+1)+jgl];
    }
  
#pragma omp parallel for
  for (int jgl = ins2; jgl >= 1; jgl--)
    {
      // find first approximation of the roots of the
      // legendre polynomial of degree kn. 
      double z = (4 * jgl - 1) * pi / (4 * kn + 2);
      double zdlx = z + 1.0 / (tan (z) * 8 * (kn * kn));
      double zxn;
      int iflag = 0;
  
      // newton iteration.
      for (int jter = 1; jter <= itemax+1; jter++)
        {
          // newton iteration step.
          double zdlk = 0.0;
          double zzdlxn = 0.0;
          double zdlldn = 0.0;
          int ik = 1;

          if (iodd == 0) 
            zdlk = 0.5 * zfnlat[0];
  
          if (iflag == 0)
            {
              for (int jn = 2 - iodd; jn <= kn; jn += 2)
                {
                  // normalised ordinary legendre polynomial == \overbar{p_n}^0 
                  zdlk = zdlk + zfnlat[ik] * cos (jn * zdlx);
                  // normalised derivative == d/d\theta(\overbar{p_n}^0) 
                  zdlldn = zdlldn - zfnlat[ik] * jn * sin (jn * zdlx);
                  ik = ik + 1;
                }
              // newton method
              double zdlmod = -zdlk / zdlldn;
              zzdlxn = zdlx + zdlmod;
              zxn = zzdlxn;
              zmod[jgl-1] = zdlmod;
            }
  
          zdlx = zzdlxn;
  
          if (iflag == 1) 
            break;

          if (abs (zmod[jgl-1]) <= zeps * 1000.) 
            iflag = 1;
        }
  
      // convert to latitude (radians)
      latgauss[jgl-1] = asin (cos (zxn));

      if (jgl <= kn/2)
        {
          int isym = kn-jgl+1;
          latgauss[isym-1] = -latgauss[jgl-1];
        }
    }
  
  delete [] zmod;
  delete [] zfnlat;
  delete [] zfn;
}

const double glgrib_geometry_gaussian::rad2deg = 180.0 / M_PI;
const double glgrib_geometry_gaussian::deg2rad = M_PI / 180.0;

#define MODULO(x, y) ((x)%(y))
#define JDLON(JLON1, JLON2) (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1))
#define JNEXT(JLON, ILOEN) ((JLON) == (ILOEN) ? 1 : (JLON)+1)
#define JPREV(JLON, ILOEN) ((JLON)-1 > 0 ? (JLON)-1 : (ILOEN))

static 
void process_lat (int jlat, int iloen1, int iloen2, 
                  int jglooff1, int jglooff2,
	          unsigned int ** p_inds_strip, int dir)  
{
// iloen1 > iloen2

  int jlon1 = 1;
  int jlon2 = 1;
  bool turn = false;
  int av1 = 0, av2 = 0;

  unsigned int * inds_strip = *p_inds_strip;
  
  for (;;)
    {
      int ica = 0, icb = 0, icc = 0;
  
      int jlon1n = dir > 0 ? JNEXT (jlon1, iloen1) : JPREV (jlon1, iloen1);
      int jlon2n = dir > 0 ? JNEXT (jlon2, iloen2) : JPREV (jlon2, iloen2);

#define AV1 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;  \
    jlon1 = jlon1n;                                                           \
    turn = turn || jlon1 == 1;                                                \
    av1++; av2 = 0;                                                           \
  } while (0)

#define AV2 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;  \
    jlon2 = jlon2n;                                                           \
    turn = turn || jlon2 == 1;                                                \
    av2++; av1 = 0;                                                           \
  } while (0)

      int idlonc = dir * JDLON (jlon1, jlon2);
      int idlonn;
      if ((jlon1n == 1) && (jlon2n != 1))
        idlonn = +1;
      else if ((jlon1n != 1) && (jlon2n == 1))
        idlonn = -1;
      else 
        idlonn = dir * JDLON (jlon1n, jlon2n);
      
      if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
        AV2;
      else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
        AV1;
      else
        abort ();
      
#define RS2 \
  do { \
    *(inds_strip++) = 0xffffffff;   \
    *(inds_strip++) = ica-1;        \
    *(inds_strip++) = icb-1;        \
    *(inds_strip++) = icc-1;        \
  } while (0)

      if (idlonc == 0)
        {
          if (av2)
            {
              abort ();
            }
          else if (av1)
            {
              RS2;
            }
        }
      else if (av2 > 1)
        {
          abort ();
        }
      else if (av1 > 1)
        {
          RS2;
        }
      else
        {
          if (inds_strip)
          *(inds_strip++) = icc-1;
        }
      
      if (turn)
        {
          if (jlon1 == 1)
            while (jlon2 != 1)
              {
                abort ();
              }
          else if (jlon2 == 1)
            while (jlon1 != 1)
              {
                int jlon1n = dir > 0 ? JNEXT (jlon1, iloen1) : JPREV (jlon1, iloen1);
                AV1;
                RS2;
              }
          break;
        }

    }


  *p_inds_strip = inds_strip;
}
  

#undef AV1
#undef AV2
#undef RS1
#undef RS2

static 
void compute_trigauss_strip (const long int Nj, const std::vector<long int> & pl, 
                             unsigned int * ind_strip,
	                     int ind_stripcnt_per_lat[], 
	                     int ind_stripoff_per_lat[])
{
  int iglooff[Nj];
  
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

#pragma omp parallel for 
  for (int jlat = 1; jlat <= Nj-1; jlat++)
    {
      int iloen1 = pl[jlat - 1];
      int iloen2 = pl[jlat + 0];
      int jglooff1 = iglooff[jlat-1] + 0;
      int jglooff2 = iglooff[jlat-1] + iloen1;
      unsigned int * inds_strip = ind_strip + ind_stripoff_per_lat[jlat-1];
  
  
      if (iloen1 == iloen2) 
        {
          *(inds_strip++) = jglooff1;
          *(inds_strip++) = jglooff2;
  
          for (int jlon1 = 1; jlon1 <= iloen1; jlon1++)
            {
              int jlon2 = jlon1;
              int ica = jglooff1 + jlon1;
              int icb = jglooff2 + jlon2;
              int icc = jglooff2 + JNEXT (jlon2, iloen2);
              int icd = jglooff1 + JNEXT (jlon1, iloen1);
              *(inds_strip++) = icd-1;
              *(inds_strip++) = icc-1;
            }
        }
      else if (iloen1 > iloen2)
        {
          process_lat (jlat, iloen1, iloen2, jglooff1, jglooff2, &inds_strip, +1);
        }
      else if (iloen1 < iloen2)
        {
          process_lat (jlat, iloen2, iloen1, jglooff2, jglooff1, &inds_strip, -1);
        }
  
      unsigned int * inds_strip_last = ind_strip 
    	                        + ind_stripoff_per_lat[jlat-1] 
    	                        + ind_stripcnt_per_lat[jlat-1];
  
      if (inds_strip >= inds_strip_last)
        abort ();
  
      for (; inds_strip < inds_strip_last; inds_strip++)
        *inds_strip = 0xffffffff;
  
    }


}

#define PRINT(a,b,c) \
  do {                                                          \
    ind[ik++] = (a)-1; ind[ik++] = (b) - 1; ind[ik++] = (c)-1;  \
  } while (0)

static 
void compute_trigauss (const long int Nj, const std::vector<long int> & pl, unsigned int * ind, 
                       const int indoff[], const int indcnt[], int triu[], int trid[])
{
  int iglooff[Nj];

  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  // No triangles above
  if (triu)
    for (int jlon = 1; jlon <= pl[0]; jlon++)
      triu[iglooff[0]+jlon-1] = -1;

  // No triangles below
  if (trid)
    for (int jlon = 1; jlon <= pl[Nj-1]; jlon++)
      trid[iglooff[Nj-1]+jlon-1] = -1;
  
#pragma omp parallel for 
  for (int jlat = 1; jlat <= Nj-1; jlat++)
    {
      int ik = 3 * indoff[jlat-1];

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
              if (triu)
                triu[icb-1] = ik / 3;
              PRINT (ica, icb, icc);
              if (trid) 
                trid[ica-1] = ik / 3;
              PRINT (ica, icc, icd);
            }
        }
      else 
        {
          int jlon1 = 1;
          int jlon2 = 1;
          bool turn = false;
          while (1)
            {
              int ica = 0, icb = 0, icc = 0;

              int jlon1n = JNEXT (jlon1, iloen1);
              int jlon2n = JNEXT (jlon2, iloen2);

#define AV1 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;  \
    if (trid) trid[ica-1] = ik / 3;                                           \
    jlon1 = jlon1n;                                                           \
    turn = turn || jlon1 == 1;                                                \
  } while (0)

#define AV2 \
  do {                                                                        \
    ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;  \
    if (triu) triu[icb-1] = ik / 3;                                           \
    jlon2 = jlon2n;                                                           \
    turn = turn || jlon2 == 1;                                                \
  } while (0)

              int idlonc = JDLON (jlon1, jlon2);
              int idlonn;
	      if ((jlon1n == 1) && (jlon2n != 1))
                idlonn = +1;
	      else if ((jlon1n != 1) && (jlon2n == 1))
                idlonn = -1;
	      else 
                idlonn = JDLON (jlon1n, jlon2n);

              if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
                AV2;
              else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
                AV1;
              else
                abort ();
         
              PRINT (ica, icb, icc);
             
              if (turn)
                {
                  if (jlon1 == 1)
                    while (jlon2 != 1)
                      {
                        int jlon2n = JNEXT (jlon2, iloen2);
                        AV2;
                        PRINT (ica, icb, icc);
                      }
                  else if (jlon2 == 1)
                    while (jlon1 != 1)
                      {
                        int jlon1n = JNEXT (jlon1, iloen1);
                        AV1;
                       PRINT (ica, icb, icc);
                      }
                  break;
                }

     
      
            }
        
        
        }


    }

#undef AV1
#undef AV2

}

int glgrib_geometry_gaussian::computeLowerTriangle (int jlat, int jlon) const
{
  if (jlat == Nj)
    return -1;
  int iloen1 = pl[jlat-1];
  int iloen2 = pl[jlat];
  int jlon1 = jlon;
  
  int jlon1n = jlon1 + 1;
  int jlon2 = 1 + ((jlon1n-1) * iloen2) / iloen1;
  
  if (JDLON (jlon1n, jlon2) == 0)
    {
      if (iloen1 < iloen2)
        jlon2 = JPREV (jlon2, iloen2);
    }
  
  int itrid = indoff_per_lat[jlat-1] + jlon1n + jlon2 - 2 - 1;

  return itrid;
}

int glgrib_geometry_gaussian::computeUpperTriangle (int jlat, int jlon) const
{
  if (jlat == 1)
    return -1;
  int iloen1 = pl[jlat-2];
  int iloen2 = pl[jlat-1];
  int jlon2 = jlon;

  int jlon2n = jlon2 + 1;
  int jlon1 = 1 + ((jlon2n-1) * iloen1) / iloen2;

  if (JDLON (jlon1, jlon2n) == 0)
    {
      if (iloen2 < iloen1)
        jlon1 = JPREV (jlon1, iloen1);
    }

  int itriu = indoff_per_lat[jlat-2] + jlon2n + jlon1 - 2 - 1;

  if (iloen1 == iloen2)
    itriu = itriu - 1;

  return itriu;
}

void glgrib_geometry_gaussian::computeTriangleVertices (int itri, int jglo[3]) const
{
  int jlat1 = 1, jlat2 = Nj, jlat;
  while (1)
    {
      jlat = (jlat1 + jlat2) / 2;

      if (jlat2 - jlat1 <= 1)
        break;

      if ((indoff_per_lat[jlat1-1] <= itri) && (itri < indoff_per_lat[jlat-1]))
        {
          jlat2 = jlat;
	}
      else
      if ((indoff_per_lat[jlat-1] <= itri) && (itri < indoff_per_lat[jlat2-1]))
        {
          jlat1 = jlat;
	}
      else
        {
          abort ();
        }

    }

  jlat1 = jlat + 0;
  jlat2 = jlat + 1;

  int iloen1 = pl[jlat1-1];
  int iloen2 = pl[jlat2-1];

  int jtri = itri - indoff_per_lat[jlat1-1];

  int jlon1, jlon2;
  int itriu2 = 0, itrid1 = 0;
  int dtri;

  if (iloen1 == iloen2)
    {
      int jlon = 1 + jtri / 2;
      int jlonn = JNEXT (jlon, iloen1);
      if (jtri % 2)
        {
          jglo[0] = jglooff[jlat1-1]+jlon -1;
          jglo[1] = jglooff[jlat2-1]+jlonn-1; 
          jglo[2] = jglooff[jlat1-1]+jlonn-1; 
        }
      else
        {
          jglo[0] = jglooff[jlat1-1]+jlon -1; 
          jglo[1] = jglooff[jlat2-1]+jlon -1;
          jglo[2] = jglooff[jlat2-1]+jlonn-1; 
        }
      return;
    }
  else if (iloen1 < iloen2)
    {
      jlon2 = 1 + (jtri * iloen2) / (iloen1 + iloen2);
      if (jlon2 == iloen2+1) jlon2 = 1;
      jlon1 = 1 + ((JNEXT (jlon2, iloen2) - 1) * iloen1) / iloen2;
      itriu2 = computeUpperTriangle (jlat2, jlon2);
      dtri = itriu2 - itri;
    }
  else if (iloen1 > iloen2)
    {
      jlon1 = 1 + (jtri * iloen1) / (iloen2 + iloen1);
      if (jlon1 == iloen1+1) jlon1 = 1;
      jlon2 = 1 + ((JNEXT (jlon1, iloen1) - 1) * iloen2) / iloen1;
      itrid1 = computeLowerTriangle (jlat1, jlon1);
      dtri = itrid1 - itri;
    }

  while (dtri < 0)
    {
      int jlon1n = JNEXT (jlon1, iloen1);
      int jlon2n = JNEXT (jlon2, iloen2);

      int idlonc = JDLON (jlon1, jlon2);
      int idlonn;
      if ((jlon1n == 1) && (jlon2n != 1))
        idlonn = +1;
      else if ((jlon1n != 1) && (jlon2n == 1))
        idlonn = -1;
      else 
        idlonn = JDLON (jlon1n, jlon2n);

      if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
        jlon2 = jlon2n;
      else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
        jlon1 = jlon1n;
      else
        abort ();
         
      dtri++;
    }
  if (dtri > 0)
    abort ();

  int jlon1n = JNEXT (jlon1, iloen1);
  int jlon2n = JNEXT (jlon2, iloen2);

  if ((jlon1n == 1) && (jlon2 == 1))
    {
      jglo[0] = jglooff[jlat1-1]+jlon1n-1; 
      jglo[1] = jglooff[jlat1-1]+jlon1 -1;
      jglo[2] = jglooff[jlat2-1]+jlon2 -1; 
      return;
    }
  else if ((jlon1 == 1) && (jlon2n == 1))
    {
      jglo[0] = jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = jglooff[jlat2-1]+jlon2n-1; 
      return;
    }

  int idlonc = JDLON (jlon1, jlon2);
  int idlonn;
  if ((jlon1n == 1) && (jlon2n != 1))
    idlonn = +1;
  else if ((jlon1n != 1) && (jlon2n == 1))
    idlonn = -1;
  else 
    idlonn = JDLON (jlon1n, jlon2n);

  if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
    {
      jglo[0] = jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = jglooff[jlat2-1]+jlon2n-1; 
    }
  else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
    {
      jglo[0] = jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = jglooff[jlat1-1]+jlon1n-1;
    }
  else
    abort ();

}

int glgrib_geometry_gaussian::size () const
{
  return jglooff[Nj-1] + pl[Nj-1];
}

glgrib_geometry_gaussian::glgrib_geometry_gaussian (int _Nj)
{
  Nj = _Nj;

  pl.resize (Nj);

  for (int jlat = 1; jlat <= Nj; jlat++)
    {   
      float lat = M_PI * (0.5 - (float)jlat / (float)(Nj + 1));
      float coslat = cos (lat);
      pl[jlat-1] = (2. * Nj * coslat);
    }   

  numberOfPoints  = 0;
  for (int i = 0; i < Nj; i++)
    numberOfPoints += pl[i];
}

glgrib_geometry_gaussian::glgrib_geometry_gaussian (glgrib_handle_ptr ghp)
{
  codes_handle * h = ghp->getCodesHandle ();

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
  if ((latitudeOfStretchingPoleInDegrees != 90.0f && longitudeOfStretchingPoleInDegrees != 0.0f)
   || (latitudeOfStretchingPoleInDegrees != 0.0f && longitudeOfStretchingPoleInDegrees != 0.0f))
    {
      rot = glm::rotate (glm::mat4 (1.0f),
                         glm::radians (90.0f-(float)latitudeOfStretchingPoleInDegrees), 
                         glm::vec3 (-sinf (glm::radians (longitudeOfStretchingPoleInDegrees)),
                                    +cosf (glm::radians (longitudeOfStretchingPoleInDegrees)),
                                    0.0f));
      rot = rot *
            glm::rotate (glm::mat4 (1.0f),
                         glm::radians (180.0f+(float)longitudeOfStretchingPoleInDegrees),
                         glm::vec3 (0.0f, 0.0f, 1.0f));
      rotated = true;
    }

  size_t pl_len;
  codes_get_long (h, "Nj", &Nj);
  codes_get_size (h, "pl", &pl_len);
  pl.resize (pl_len);
  codes_get_long_array (h, "pl", pl.data (), &pl_len);

  numberOfPoints  = 0;
  for (int i = 0; i < Nj; i++)
    numberOfPoints += pl[i];
}

static
void roll3 (int jglo[3])
{
  int l = std::min_element (jglo, jglo + 3) - &jglo[0];

  if (l == 1)
    {
      int t = jglo[0]; jglo[0] = jglo[1]; jglo[1] = jglo[2]; jglo[2] = t;
    }
  if (l == 2)
    {
      int t = jglo[0]; jglo[0] = jglo[2]; jglo[2] = jglo[1]; jglo[1] = t;
    }
}
        
void glgrib_geometry_gaussian::checkTriangleComputation () const
{

  if (0){
  printf (" %8s %8s\n", "jlat", "pl");
  for (int i = 0; i < Nj; i++)
    printf (" %8d %8d\n", i+1, pl[i]);
  }

  printf ("----ITRID----\n");

  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      int pr = 1;

      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++)
        {
          int itrid = computeLowerTriangle (jlat, jlon);
          int jglo = jglooff[jlat-1]+jlon-1;

          if (itrid != trid[jglo])
	    {
              if (pr) {
              printf (" jlat = %8d\n", jlat);
              printf (" %8s %8s %8s\n", "jlon", "itrid", "itrid");
	      pr = 0;
	      }
              printf (" %8d %8d %8d %1d\n", jlon, itrid, trid[jglo], 
                      itrid == trid[jglo]);
	    }
     
        }

    }

  printf ("----ITRIU----\n");

  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      int pr = 1;

      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++)
        {
          int itriu = computeUpperTriangle (jlat, jlon);
          int jglo = jglooff[jlat-1]+jlon-1;

          if (itriu != triu[jglo])
	    {
              if (pr) {
              printf (" jlat = %8d\n", jlat);
              printf (" %8s %8s %8s\n", "jlon", "itriu", "itriu");
	      pr = 0;
	      }
              printf (" %8d %8d %8d %1d\n", jlon, itriu, triu[jglo], 
                      itriu == triu[jglo]);
	    }
     
        }

    }

  printf ("-----IND-----\n");

  for (int it = 0; it < numberOfTriangles; it++)
    {
      int jglo0[3];
      int jglo1[3];
      for (int i = 0; i < 3; i++)
        jglo0[i] = ind[3*it+i];
 
      computeTriangleVertices (it, jglo1);

      roll3 (jglo0);
      roll3 (jglo1);

      int diff = ((jglo0[0] != jglo1[0]) ||
                  (jglo0[1] != jglo1[1]) ||
                  (jglo0[2] != jglo1[2]));
      if (diff)
      printf (">> %8d -> %8d %8d %8d | %8d %8d %8d | %s\n", it,
              jglo0[0], jglo0[1], jglo0[2],
              jglo1[0], jglo1[1], jglo1[2], diff ? "X" : " ");
    }

end:
  return;
}

void glgrib_geometry_gaussian::setup (glgrib_handle_ptr ghp, const glgrib_options_geometry & opts, const float orography)
{
  codes_handle * h = ghp ? ghp->getCodesHandle () : NULL;

  bool orog = (orography > 0.0f) && (h != NULL);

  if (orog)
    {
      heightbuffer = new_glgrib_opengl_buffer_ptr (numberOfPoints * sizeof (float));

      float * height = (float *)heightbuffer->map ();

      double vmin, vmax, vmis;
      double * v = new double[numberOfPoints];
      size_t v_len = numberOfPoints;

      codes_get_double_array (h, "values", v, &v_len);
      codes_get_double (h, "maximum",      &vmax);
      codes_get_double (h, "minimum",      &vmin);
      codes_get_double (h, "missingValue", &vmis);

#pragma omp parallel for
      for (int jglo = 0; jglo < numberOfPoints; jglo++)
        height[jglo] = v[jglo] == vmis ? 0.0f : orography * v[jglo] / vmax;

      delete [] v;

      heightbuffer->unmap ();
    }

  // Compute number of triangles
  numberOfTriangles = 0;
  for (int jlat = 1; jlat < Nj; jlat++)
    numberOfTriangles += pl[jlat-1] + pl[jlat];
  
  indcnt_per_lat = new int[Nj];
  indoff_per_lat = new int[Nj];
  // Compute number of triangles per latitude
  indoff_per_lat[0] = 0;
  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      if (jlat < Nj)
        indcnt_per_lat[jlat-1] = pl[jlat-1] + pl[jlat];
      if (jlat > 1)
        indoff_per_lat[jlat-1] = indoff_per_lat[jlat-2] + indcnt_per_lat[jlat-2];
    }


  if (! opts.triangle_strip.on)
    {
      ind  = new unsigned int[3 * numberOfTriangles]; 
      triu = new int[numberOfPoints]; 
      trid = new int[numberOfPoints]; 
      // Generation of triangles
      compute_trigauss (Nj, pl, ind, indoff_per_lat, indcnt_per_lat, triu, trid);
      elementbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfTriangles * sizeof (ind[0]), ind);
    }
  else
    {
      int * ind_stripcnt_per_lat  = new int[Nj+1];
      int * ind_stripoff_per_lat  = new int[Nj+1];

      ind_stripoff_per_lat[0] = 0;
      for (int jlat = 1; jlat <= Nj+1; jlat++)
        {
          if (jlat < Nj) // k1^k2 < (k1 - k2); k1^k2 is the number of possible restarts
            ind_stripcnt_per_lat[jlat-1] = pl[jlat-1] + pl[jlat] 
                     + 4 * (2 + abs (pl[jlat-1] - pl[jlat]));
          if (jlat > 1)
            ind_stripoff_per_lat[jlat-1] = ind_stripcnt_per_lat[jlat-2] 
                     + ind_stripoff_per_lat[jlat-2];
        }

  
      ind_strip_size = 0;
      for (int jlat = 1; jlat < Nj; jlat++)
        ind_strip_size += ind_stripcnt_per_lat[jlat-1];

      elementbuffer = new_glgrib_opengl_buffer_ptr (ind_strip_size * sizeof (unsigned int));
      unsigned int * ind_strip = (unsigned int*)elementbuffer->map ();

      compute_trigauss_strip (Nj, pl, ind_strip, ind_stripcnt_per_lat, ind_stripoff_per_lat); 

      delete [] ind_stripcnt_per_lat;
      delete [] ind_stripoff_per_lat;

      elementbuffer->unmap ();

    }


  latgauss = new double[Nj]; 
  // Compute Gaussian latitudes
  compute_latgauss (Nj, latgauss);
      
  vertexbuffer = new_glgrib_opengl_buffer_ptr (2 * numberOfPoints * sizeof (float));
  float * lonlat = (float *)vertexbuffer->map ();

  int iglooff[Nj];
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  // OpenMP generation of coordinates
#pragma omp parallel for 
  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      float coordy = latgauss[jlat-1];
      float lat;

      if (rotated)
        {
          float sincoordy = sin (coordy);
          lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
        }
      else
        {
          lat = coordy;
        }

      float coslat, sinlat;

      if (rotated)
        {
          coslat = cos (lat); 
	  sinlat = sin (lat);
	}

      int jglo = iglooff[jlat-1];

      for (int jlon = 1; jlon <= pl[jlat-1]; jlon++, jglo++)
        {

          float coordx = 2. * M_PI * (float)(jlon-1) / (float)pl[jlat-1];
          float lon = coordx;

	  if (! rotated)
            {
              lonlat[2*jglo+0] = lon;
              lonlat[2*jglo+1] = lat;
            }
          else
            {
              float coslon = cos (lon); float sinlon = sin (lon);

              float X = coslon * coslat;
              float Y = sinlon * coslat;
              float Z =          sinlat;

              glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
              XYZ = rot * XYZ;

              lonlat[2*jglo+0] = atan2 (XYZ.y, XYZ.x);
              lonlat[2*jglo+1] = asin (XYZ.z);

	    }

        }
    }

  lonlat = NULL;
  vertexbuffer->unmap ();

  jglooff.resize (Nj + 1);

  jglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj + 1; jlat++)
    jglooff[jlat-1] = jglooff[jlat-2] + pl[jlat-2];

  if (opts.check.on)
    {
      checkTriangleComputation ();
    }
}

glgrib_geometry_gaussian::~glgrib_geometry_gaussian ()
{
  if (ind)
    delete [] ind;
  if (triu)
    delete [] triu;
  if (trid)
    delete [] trid;
  if (latgauss)
    delete [] latgauss;
  if (indoff_per_lat)
    delete [] indoff_per_lat;
  if (indcnt_per_lat)
    delete [] indcnt_per_lat;
  ind = NULL;
  triu = NULL;
  trid = NULL;
  latgauss = NULL;
  indoff_per_lat = NULL;
  indcnt_per_lat = NULL;
}

void glgrib_geometry_gaussian::latlon2coordxy (float lat, float lon, 
                                               float & coordx, float & coordy) const
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

  coordx = lon;
  sinlat = sin (lat);
  coordy = asin ((-omc2 + sinlat * opc2) / (opc2 - sinlat * omc2));
}

int glgrib_geometry_gaussian::latlon2jlatjlon (float lat, float lon, int & jlat, int & jlon) const
{
  float coordx, coordy;

  latlon2coordxy (lat, lon, coordx, coordy);

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  jlat = round ((0.5 - coordy / M_PI) * (Nj + 1)); // First approximation
  jlat = std::max (1, std::min (jlat, (int)Nj));

  float dlat = fabs (latgauss[jlat-1] - coordy);

  while (1)
    {
      bool u = false;
      auto lookat = [&u,&jlat,&dlat,coordy,this] (int jlat1)
      {
        float dlat1 = fabs (this->latgauss[jlat1-1] - coordy);
	if (dlat1 < dlat)
          {
            dlat = dlat1; jlat = jlat1; u = true; 
	  }
	return;
      };
      if (1 < jlat)
        lookat (jlat-1);
      if (jlat < Nj)
	lookat (jlat+1);
      if (! u)
        break;
    }

  if (coordx < 0.0f)
    coordx += 2.0f * M_PI;
  jlon = round (pl[jlat-1] * (coordx / (2. * M_PI)));

  jlat = jlat - 1; // Start at zero

  return 0;
}

int glgrib_geometry_gaussian::latlon2index (float lat, float lon) const
{
  int jlat, jlon;
  latlon2jlatjlon (lat, lon, jlat, jlon);
  return jglooff[jlat] + jlon;
}

std::string glgrib_geometry_gaussian::md5 () const
{
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_CTX c;
  MD5_Init (&c);
  
  MD5_Update (&c, &Nj, sizeof (Nj));
  MD5_Update (&c, pl.data (), Nj * sizeof (pl[0]));
  MD5_Update (&c, &stretchingFactor, sizeof (stretchingFactor));
  MD5_Update (&c, &latitudeOfStretchingPoleInDegrees, sizeof (latitudeOfStretchingPoleInDegrees));
  MD5_Update (&c, &longitudeOfStretchingPoleInDegrees, sizeof (longitudeOfStretchingPoleInDegrees));
  MD5_Final (out, &c);

  return md5string (out);
}


bool glgrib_geometry_gaussian::isEqual (const glgrib_geometry & other) const
{
  const float epsilon = 1E-4;
  try
    {
      const glgrib_geometry_gaussian & g = dynamic_cast<const glgrib_geometry_gaussian &>(other);
      return (Nj == g.Nj) && (fabs (stretchingFactor - g.stretchingFactor) < epsilon) &&
             (fabs (latitudeOfStretchingPoleInDegrees - g.latitudeOfStretchingPoleInDegrees) < epsilon) &&
	     (fabs (longitudeOfStretchingPoleInDegrees - g.longitudeOfStretchingPoleInDegrees) < epsilon) &&
	     (memcmp (pl.data (), g.pl.data (), sizeof (pl[0]) * Nj) == 0);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void glgrib_geometry_gaussian::applyUVangle (float * angle) const 
{
  if (rotated)
    {
      float coslon0 = cos (deg2rad * longitudeOfStretchingPoleInDegrees);
      float sinlon0 = sin (deg2rad * longitudeOfStretchingPoleInDegrees);
      float coslat0 = cos (deg2rad * latitudeOfStretchingPoleInDegrees);
      float sinlat0 = sin (deg2rad * latitudeOfStretchingPoleInDegrees);
      glm::vec3 xyz0 = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);
   
#pragma omp parallel for 
      for (int jlat = 0; jlat < Nj; jlat++)
        {
          float coordy = latgauss[jlat];
          float sincoordy = sin (coordy);
          float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
          float coslat = cos (lat); float sinlat = sin (lat);
          for (int jlon = 0; jlon < pl[jlat]; jlon++)
            {
              int jglo = jglooff[jlat] + jlon;

              float coordx = 2. * M_PI * (float)jlon / (float)pl[jlat];
              float lon = coordx;
              float coslon = cos (lon); float sinlon = sin (lon);
  
              glm::vec4 XYZ = rot * glm::vec4 (coslon * coslat, sinlon * coslat, sinlat, 0.0f);
              glm::vec3 xyz = glm::vec3 (XYZ.x, XYZ.y, XYZ.z);
  
              glm::vec3 u1 = glm::normalize (glm::cross (xyz0 - xyz, xyz));
              glm::vec3 u0 = glm::normalize (glm::cross (glm::vec3 (0.0f, 0.0f, 1.0f), xyz));
              glm::vec3 v0 = glm::cross (xyz, u0);
              float u0u1 = glm::dot (u0, u1);
              float v0u1 = glm::dot (v0, u1);
  
              angle[jglo] += rad2deg * atan2 (v0u1, u0u1);

              if (angle[jglo] < -180.0f)
                angle[jglo] += 360.0f;
              if (angle[jglo] > +180.0f)
                angle[jglo] -= 360.0f;
	    }
        }
    }
  return;
}

void glgrib_geometry_gaussian::sample (unsigned char * p, const unsigned char p0, const int level) const
{
  int lat_stride = (float)Nj / (float)level;
  if (lat_stride == 0)
    lat_stride = 1;
#pragma omp parallel for 
  for (int jlat = 0; jlat < Nj; jlat++)
    {
      float lat = M_PI * (0.5 - (float)(jlat+1) / (float)(Nj + 1));
      int lon_stride = lat_stride / cos (lat);
      if (lon_stride == 0)
        lon_stride = 1;
      for (int jlon = 0; jlon < pl[jlat]; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jglooff[jlat]+jlon] = p0;
    }
}

glgrib_geometry_gaussian::jlonlat_t glgrib_geometry_gaussian::jlonlat (int jglo) const 
{
  int jlat1 = 0, jlat2 = Nj, jlat;
  while (jlat2 != jlat1 + 1)
    {
      int jlatm = (jlat1 + jlat2) / 2;
      if ((jglooff[jlat1] <= jglo) && (jglo < jglooff[jlatm]))
        jlat2 = jlatm;
      else if ((jglooff[jlatm] <= jglo) && (jglo < jglooff[jlat2]))
        jlat1 = jlatm;
    }
  jlat = 1 + jlat1;
  int jlon = 1 + jglo - jglooff[jlat-1];
  return jlonlat_t (jlon, jlat);
}


void glgrib_geometry_gaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], jlonlat_t jlonlat_[3]) const
{
  int jgloABC[3];
  getTriangleVertices (it, jgloABC);
  const int jgloA = jgloABC[0]; 
  const int jgloB = jgloABC[1]; 
  const int jgloC = jgloABC[2]; 
  
  jlonlat_t jlonlatA = jlonlat (jgloA);
  jlonlat_t jlonlatB = jlonlat (jgloB);
  jlonlat_t jlonlatC = jlonlat (jgloC);
  
  int jglo0, jglo1, jglo2;
  jlonlat_t jlonlat0, jlonlat1, jlonlat2;
  
  if (jlonlatA.jlat == jlonlatB.jlat)
    {
      jglo0    = jgloA;    jglo1    = jgloB;    jglo2    = jgloC;    
      jlonlat0 = jlonlatA; jlonlat1 = jlonlatB; jlonlat2 = jlonlatC;
    }
  else 
  if (jlonlatA.jlat == jlonlatC.jlat)
    {
      jglo0    = jgloA;    jglo1    = jgloC;    jglo2    = jgloB;    
      jlonlat0 = jlonlatA; jlonlat1 = jlonlatC; jlonlat2 = jlonlatB;
    }
  else 
  if (jlonlatB.jlat == jlonlatC.jlat)
    {
      jglo0    = jgloB;    jglo1    = jgloC;    jglo2    = jgloA;    
      jlonlat0 = jlonlatB; jlonlat1 = jlonlatC; jlonlat2 = jlonlatA;
    }
  
  bool up = jlonlat2.jlat < jlonlat0.jlat;
  
  int ntri = pl[jlonlat0.jlat-1] + pl[jlonlat2.jlat-1];               // Number of triangles on this row
  int lat1 = up ? jlonlat2.jlat : jlonlat0.jlat;
  int otri = lat1 == 1 ? 0 : jglooff[lat1] * 2 - pl[lat1-1] - pl[0];  // Offset of triangles counting
  int ktri = it - otri;                                               // Rank of current triangle in current row
  int Ltri = ktri == 0      ? ntri-1 : ktri-1; Ltri += otri;          // Rank of left triangle
  int Rtri = ktri == ntri-1 ?      0 : ktri+1; Rtri += otri;          // Rank of right triangle
  int Vtri = up 
           ? getLowerTriangle (jglo0, jlonlat0) 
           : getUpperTriangle (jglo0, jlonlat0);                      // Rank of triangle above of under segment 01

  int itr01 = Vtri, itr12 = Rtri, itr20 = Ltri;
  
  jglo[0] = jglo0; jglo[1] = jglo1; jglo[2] = jglo2;
  itri[0] = itr01; itri[1] = itr12; itri[2] = itr20;

  jlonlat_[0] = jlonlat0;
  jlonlat_[1] = jlonlat1;
  jlonlat_[2] = jlonlat2;
}

int glgrib_geometry_gaussian::getUpperTriangle (int jglo, const jlonlat_t & jlonlat) const
{
  if (triu)
    return triu[jglo];
  else
    return computeUpperTriangle (jlonlat.jlat, jlonlat.jlon);
}

int glgrib_geometry_gaussian::getLowerTriangle (int jglo, const jlonlat_t & jlonlat) const
{
  if (trid)
    return trid[jglo];
  else
    return computeLowerTriangle (jlonlat.jlat, jlonlat.jlon);
}

void glgrib_geometry_gaussian::getTriangleVertices (int it, int jglo[3]) const
{
  if (ind)
    {
      jglo[0] = ind[3*it+0]; jglo[1] = ind[3*it+1]; jglo[2] = ind[3*it+2];
    }
  else
    {
      computeTriangleVertices (it, jglo);
    }
}

void glgrib_geometry_gaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
{
  jlonlat_t jlonlat[3];

  getTriangleNeighbours (it, jglo, itri, jlonlat);

  xyz[0] = jlonlat2xyz (jlonlat[0]);
  xyz[1] = jlonlat2xyz (jlonlat[1]);
  xyz[2] = jlonlat2xyz (jlonlat[2]);
}

void glgrib_geometry_gaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec2 merc[3]) const
{
  jlonlat_t jlonlat[3];

  getTriangleNeighbours (it, jglo, itri, jlonlat);

  merc[0] = jlonlat2merc (jlonlat[0]);
  merc[1] = jlonlat2merc (jlonlat[1]);
  merc[2] = jlonlat2merc (jlonlat[2]);
}


bool glgrib_geometry_gaussian::triangleIsEdge (int it) const
{
  int jgloABC[3];
  getTriangleVertices (it, jgloABC);
  const int jgloA = jgloABC[0]; 
  const int jgloB = jgloABC[1]; 
  const int jgloC = jgloABC[2]; 

  if ((jgloA >= jglooff[1]) && (jgloB >= jglooff[1]) && (jgloC >= jglooff[1]) 
   && (jgloA < jglooff[Nj-1]) && (jgloB < jglooff[Nj-1]) && (jgloC < jglooff[Nj-1]))
    return false;
  
  int jglo[3]; 
  int itri[3]; 
  glm::vec3 xyz[3];
  getTriangleNeighbours (it, jglo, itri, xyz);

  return itri[0] < 0;
}


void glgrib_geometry_gaussian::applyNormScale (float * data) const 
{
  if (stretchingFactor == 1.0f)
    return;
#pragma omp parallel for 
  for (int jlat = 0; jlat < Nj; jlat++)
    {
      float coordy = latgauss[jlat];
      float sincoordy = sin (coordy);
      float N = 1.0f / sqrt ((opc2 + sincoordy * omc2) * (opc2 + sincoordy * omc2) 
                           / (opc2 * opc2 - omc2 * omc2));
      N = 1.0f / N;
      for (int jlon = 0; jlon < pl[jlat]; jlon++)
        {
          int jglo = jglooff[jlat] + jlon;
          data[jglo] = N * data[jglo];
        }
    }
}

void glgrib_geometry_gaussian::sampleTriangle (unsigned char * s, const unsigned char s0, const int level) const
{
  int lev = std::max (1, level);
  int itrioff = 0;
  for (int jlat = 1; jlat <= Nj-1; jlat++)
    {
      int ntri = pl[jlat-1] + pl[jlat-1];   // Triangles on this row
      for (int jtri = 1; jtri <= ntri; jtri++)
        if (((jlat - 1) % lev == 0) && ((jtri - 1) % (2 * lev) == 0))
          s[itrioff+jtri-1] = s0;
      itrioff += ntri;
    }
}


int glgrib_geometry_gaussian::getTriangle (float lon, float lat) const
{
  int jlat, jlon;

  float coordx, coordy;

  latlon2coordxy (lat, lon, coordx, coordy);

  if ((coordy > latgauss[0]) || (latgauss[Nj-1] > coordy))
    return -1;

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  jlat = round ((0.5 - coordy / M_PI) * (Nj + 1)); // First approximation
  jlat = std::max (1, std::min (jlat, (int)Nj));

  float dlat;

  while ((dlat = coordy - latgauss[jlat-1]) > 0.0f)
    {
      jlat--;
      if (jlat <= 0)
        return -1;
    }

  while (1)
    {
      bool u = false;
      auto lookat = [&u,&jlat,&dlat,coordy,this] (int jlat1)
      {
        float dlat1 = coordy - this->latgauss[jlat1-1];
	if (fabs (dlat1) < fabs (dlat) && (dlat1 <= 0.0f))
          {
            dlat = dlat1; jlat = jlat1; u = true; 
	  }
	return;
      };
      if (1 < jlat)
        lookat (jlat-1);
      if (jlat < Nj)
	lookat (jlat+1);
      if (! u)
        break;
    }


  jlat++;

  if (coordx < 0.0f)
    coordx += 2.0f * M_PI;
  jlon = (int) (pl[jlat-1] * (coordx / (2. * M_PI)));

  jlat = jlat - 1; // Start at zero

  std::cout << " jlon, jlat = " << jlon << ", " << jlat << std::endl;

  int jglo = jglooff[jlat] + jlon;
  std::cout << " it = " << triu[jglo] << std::endl;

  return 0;
}

glm::vec2 glgrib_geometry_gaussian::xyz2conformal (const glm::vec3 & xyz) const
{
  float lon = atan2 (xyz.y, xyz.x);
  float lat = asin (xyz.z);
  return glm::vec2 (lon, log (tan (M_PI / 4.0f + lat / 2.0f)));
}

glm::vec3 glgrib_geometry_gaussian::conformal2xyz (const glm::vec2 & merc) const
{
  float coordx = merc.x;
  float coordy = 2.0f * atan (exp (merc.y)) - M_PI / 2.0f;
  float sincoordy = sin (coordy);

  float lat = asin ((omc2 + sincoordy * opc2) / (opc2 + sincoordy * omc2));
  float lon = coordx;

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  float X = coslon * coslat;
  float Y = sinlon * coslat;
  float Z =          sinlat;

  if (! rotated)
    return glm::vec3 (X, Y, Z);
  
  glm::vec4 XYZ = glm::vec4 (X, Y, Z, 0.0f);
  XYZ = rot * XYZ;
 
  return glm::vec3 (XYZ.x, XYZ.y, XYZ.z);
}

glm::vec2 glgrib_geometry_gaussian::conformal2latlon (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * atan (exp (merc.y)) - M_PI / 2.0f;
  return glm::vec2 (glm::degrees (lon), glm::degrees (lat));
}

void glgrib_geometry_gaussian::fixPeriodicity (const glm::vec2 & M, glm::vec2 * P, int n) const
{
  // Fix periodicity issue
  for (int i = 0; i < n; i++)
    {
      while (M.x - P[i].x > M_PI)
        P[i].x += 2.0f * M_PI;
      while (P[i].x - M.x > M_PI)
        P[i].x -= 2.0f * M_PI;
    }
}
