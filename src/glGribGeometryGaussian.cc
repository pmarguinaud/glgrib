#include "glGribGeometryGaussian.h"
#include "glGribTrigonometry.h"
#include "glGribFitPolynomial.h"

#include <glm/gtc/matrix_transform.hpp>
#include <openssl/md5.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <algorithm>
#include <exception>
#include <functional>


namespace
{

void computeLatgauss (int kn, glGrib::BufferPtr<double> & latgauss)
{
  const double pi = M_PI;
  const int itemax = 20;
  const double zeps = std::numeric_limits <double>::epsilon ();
  
  glGrib::Buffer<double> zfn ((kn+1)*(kn+1));
  glGrib::Buffer<double> zfnlat (kn/2+2);
  glGrib::Buffer<double> zmod (kn);

  //     ------------------------------------------------------------------
  //        1. initialization.
  //           ---------------
  // 
  //        1.1 calculation of zfnlat.
  //            (fourier coefficients of series expansion for
  //            the ordinary legendre polynomials).
  // belousov, swarztrauber use zfn(0,0)=std::sqrt(2._jprb)
  // ifs normalisation chosen to be 0.5*integral(pnm**2) = 1 

  zfn[0] = 2.;
#pragma omp parallel for
  for (int jn = 1; jn <= kn; jn++)
    {
      double zfnn = zfn[0];
      for (int jgl = 1; jgl <= jn; jgl++)
        zfnn = zfnn * std::sqrt (1. - 0.25 / (jgl * jgl));
      int iodd = jn % 2;
      zfn[jn*(kn+1)+jn] = zfnn;
      for (int jgl = 2; jgl <= jn-iodd; jgl += 2)
        zfn[jn*(kn+1)+jn-jgl] = zfn[jn*(kn+1)+jn-jgl+2] 
         * static_cast<double>((jgl-1)*(2*jn-jgl+2)) / static_cast<double>(jgl*(2*jn-jgl+1));
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
      double zdlx = z + 1.0 / (std::tan (z) * 8 * (kn * kn));
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
                  zdlk = zdlk + zfnlat[ik] * std::cos (jn * zdlx);
                  // normalised derivative == d/d\theta(\overbar{p_n}^0) 
                  zdlldn = zdlldn - zfnlat[ik] * jn * std::sin (jn * zdlx);
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
      latgauss[jgl-1] = std::asin (std::cos (zxn));

      if (jgl <= kn/2)
        {
          int isym = kn-jgl+1;
          latgauss[isym-1] = -latgauss[jgl-1];
        }
    }
  
}

int MODULO (int x, int y)
{
  return ((x)%(y));
}


int JNEXT (int JLON, int ILOEN)
{
  return ((JLON) == (ILOEN) ? 1 : (JLON)+1);
}

int JPREV (int JLON, int ILOEN)
{
   return ((JLON)-1 > 0 ? (JLON)-1 : (ILOEN));
}


void processLat (int jlat, int iloen1, int iloen2, 
                  int jglooff1, int jglooff2,
	          unsigned int ** p_inds_strip, int dir)  
{
// iloen1 > iloen2

  auto JDLON = [&iloen1, &iloen2] (int JLON1, int JLON2)
  {
    return (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1));
  };

  int jlon1 = 1;
  int jlon2 = 1;
  bool turn = false;
  int av1 = 0, av2 = 0;

  unsigned int * inds_strip = *p_inds_strip;

  
  for (;;)
    {
      int ica = 0, icb = 0, icc = 0;
  
      int jlon1n = JNEXT (jlon1, iloen1);
      int jlon2n = JNEXT (jlon2, iloen2);

      auto AV1 = [&] ()
      {
        ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;  
        jlon1 = jlon1n;                                                           
        turn = turn || jlon1 == 1;                                                
        av1++; av2 = 0;                                                          
      };

      auto AV2 = [&] ()
      {
        ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;  
        jlon2 = jlon2n;                                                           
        turn = turn || jlon2 == 1;                                                
        av2++; av1 = 0;                                                           
      };

      int idlonc = JDLON (jlon1, jlon2);
      int idlonn;
      if ((jlon1n == 1) && (jlon2n != 1))
        idlonn = +1;
      else if ((jlon1n != 1) && (jlon2n == 1))
        idlonn = -1;
      else 
        idlonn = JDLON (jlon1n, jlon2n);

      if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
        AV2 ();
      else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
        AV1 ();
      else
        abort ();
      
      auto RS2 = [&] ()
      {
        *(inds_strip++) = glGrib::OpenGL::restart;   
        *(inds_strip++) = ica-1;        
        *(inds_strip++) = icb-1;        
        *(inds_strip++) = icc-1;        
      };

      auto ST1 = [&] () 
      {
        *(inds_strip++) = glGrib::OpenGL::restart;  
        *(inds_strip++) = ica-1+iloen1-1;
        *(inds_strip++) = ica-1;
        *(inds_strip++) = icb-1;
        *(inds_strip++) = icc-1;
      };
  
      auto RS1 = [&] ()
      {
        *(inds_strip++) = icc-1;
        *(inds_strip++) = glGrib::OpenGL::restart;  
        *(inds_strip++) = icb-1;
        *(inds_strip++) = icc-1;
      };

      if (dir > 0)
        {
          if (idlonc == 0) 
            {
              if (av2)
                {
                  abort ();
                }
              else if (av1)
                {
                  RS2 ();
                }
            }
          else if (av2 > 1)
            {
              abort ();
            }
          else if (av1 > 1)
            {
              RS2 ();
            }
          else
            {
              *(inds_strip++) = icc-1;
            }
        }
      else
        {
          bool first = inds_strip == *p_inds_strip;
          if (first) 
            {
              ST1 ();
            }
          else
            {
              if (av2 > 1)
                {
                  abort ();
                }
              else if (av1 > 1)
                {
                  RS1 ();
                }
              else
                {
                  *(inds_strip++) = icc-1;
                }

            }
        }
      
      if (turn)
        {
          if (dir > 0)
            {
              if (jlon1 == 1)
                while (jlon2 != 1)
                  {
                    abort ();
                  }
              else if (jlon2 == 1) 
                while (jlon1 != 1)
                  {
                    jlon1n = JNEXT (jlon1, iloen1);
                    AV1 ();
                    RS2 ();
                  }
            }
          break;
        }

    }

  *p_inds_strip = inds_strip;
}
  
void computeTrigaussStrip 
  (const long int Nj, const std::vector<long int> & pl, 
   glGrib::OpenGLBuffer<unsigned int>::Mapping & ind_stripm,
   glGrib::BufferPtr<int> & ind_stripcnt_per_lat, 
   glGrib::BufferPtr<int> & ind_stripoff_per_lat)
{
  int iglooff[Nj];

  unsigned int * ind_strip = &ind_stripm[0];
  

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
          *(inds_strip++) = jglooff1; // A
          for (int jlon1 = 1; jlon1 <= iloen1; jlon1++)
            {
              int jlon2 = jlon1;
              int icc = jglooff2 + JNEXT (jlon2, iloen2); // C
              int icd = jglooff1 + JNEXT (jlon1, iloen1); // D
              *(inds_strip++) = icc-1; 
              *(inds_strip++) = icd-1; 
            }
          *(inds_strip++) = jglooff2 + 1; 
        }
      else if (iloen1 > iloen2)
        {
          processLat (jlat, iloen1, iloen2, jglooff1, jglooff2, 
                      &inds_strip, +1);
        }
      else if (iloen1 < iloen2)
        {
          processLat (jlat, iloen2, iloen1, jglooff2, jglooff1, 
                      &inds_strip, -1);
        }
  
      unsigned int * inds_strip_last = ind_strip 
    	                        + ind_stripoff_per_lat[jlat-1] 
    	                        + ind_stripcnt_per_lat[jlat-1];
  
      if (inds_strip >= inds_strip_last)
        abort ();
  
      for (; inds_strip < inds_strip_last; inds_strip++)
        *inds_strip = glGrib::OpenGL::restart;
  
    }


}



void computeTrigauss (const long int Nj, const std::vector<long int> & pl, 
                      glGrib::BufferPtr<unsigned int> & ind, 
                      const glGrib::BufferPtr<int> & indoff, 
                      const glGrib::BufferPtr<int> & indcnt, 
                      glGrib::BufferPtr<int> & triu, 
                      glGrib::BufferPtr<int> & trid)
{
  int iglooff[Nj];

  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  // No triangles above
  if (triu.allocated ())
    for (int jlon = 1; jlon <= pl[0]; jlon++)
      triu[iglooff[0]+jlon-1] = -1;

  // No triangles below
  if (trid.allocated ())
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
     
      auto JDLON = [&iloen1, &iloen2] (int JLON1, int JLON2)
      {
        return (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1));
      };

      auto PRINT = [&] (int a,int b,int c) 
      {
        ind[ik++] = (a)-1; ind[ik++] = (b) - 1; ind[ik++] = (c)-1;  
      };

      if (iloen1 == iloen2) 
        {
          for (int jlon1 = 1; jlon1 <= iloen1; jlon1++)
            {
              int jlon2 = jlon1;
              int ica = jglooff1 + jlon1;
              int icb = jglooff2 + jlon2;
              int icc = jglooff2 + JNEXT (jlon2, iloen2);
              int icd = jglooff1 + JNEXT (jlon1, iloen1);
              if (triu.allocated ())
                triu[icb-1] = ik / 3;
              PRINT (ica, icb, icc);
              if (trid.allocated ()) 
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

              auto AV1 = [&] ()
              {
                ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff1 + jlon1n;  
                if (trid.allocated ()) trid[ica-1] = ik / 3;                                           
                jlon1 = jlon1n;                                                           
                turn = turn || jlon1 == 1;                                                
              };

              auto AV2 = [&] ()
              {
                ica = jglooff1 + jlon1; icb = jglooff2 + jlon2; icc = jglooff2 + jlon2n;  
                if (triu.allocated ()) triu[icb-1] = ik / 3;                                           
                jlon2 = jlon2n;                                                           
                turn = turn || jlon2 == 1;                                                
              };

              int idlonc = JDLON (jlon1, jlon2);
              int idlonn;
	      if ((jlon1n == 1) && (jlon2n != 1))
                idlonn = +1;
	      else if ((jlon1n != 1) && (jlon2n == 1))
                idlonn = -1;
	      else 
                idlonn = JDLON (jlon1n, jlon2n);

              if (idlonn > 0 || ((idlonn == 0) && (idlonc > 0)))
                AV2 ();
              else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
                AV1 ();
              else
                abort ();
         
              PRINT (ica, icb, icc);
             
              if (turn)
                {
                  if (jlon1 == 1)
                    while (jlon2 != 1)
                      {
                        jlon2n = JNEXT (jlon2, iloen2);
                        AV2 ();
                        PRINT (ica, icb, icc);
                      }
                  else if (jlon2 == 1)
                    while (jlon1 != 1)
                      {
                        jlon1n = JNEXT (jlon1, iloen1);
                        AV1 ();
                        PRINT (ica, icb, icc);
                      }
                  break;
                }
            }
        }
    }

}

}

int glGrib::GeometryGaussian::computeLowerTriangle (int jlat, int jlon) const
{
  if (jlat == grid_gaussian.Nj)
    return -1;
  int iloen1 = grid_gaussian.pl[jlat-1];
  int iloen2 = grid_gaussian.pl[jlat];
  int jlon1 = jlon;
  
  auto JDLON = [&iloen1, &iloen2] (int JLON1, int JLON2)
  {
    return (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1));
  };

  int jlon1n = jlon1 + 1;
  int jlon2 = 1 + ((jlon1n-1) * iloen2) / iloen1;
  
  if (JDLON (jlon1n, jlon2) == 0)
    {
      if (iloen1 < iloen2)
        jlon2 = JPREV (jlon2, iloen2);
    }
  
  int itrid = grid_gaussian.indoff_per_lat[jlat-1] + jlon1n + jlon2 - 2 - 1;

  return itrid;
}

int glGrib::GeometryGaussian::computeUpperTriangle (int jlat, int jlon) const
{
  if (jlat == 1)
    return -1;
  int iloen1 = grid_gaussian.pl[jlat-2];
  int iloen2 = grid_gaussian.pl[jlat-1];
  int jlon2 = jlon;

  auto JDLON = [&iloen1, &iloen2] (int JLON1, int JLON2)
  {
    return (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1));
  };

  int jlon2n = jlon2 + 1;
  int jlon1 = 1 + ((jlon2n-1) * iloen1) / iloen2;

  if (JDLON (jlon1, jlon2n) == 0)
    {
      if (iloen2 < iloen1)
        jlon1 = JPREV (jlon1, iloen1);
    }

  int itriu = grid_gaussian.indoff_per_lat[jlat-2] + jlon2n + jlon1 - 2 - 1;

  if (iloen1 == iloen2)
    itriu = itriu - 1;

  return itriu;
}

void glGrib::GeometryGaussian::computeTriangleVertices (int itri, int jglo[3]) const
{
  int jlat1 = 1, jlat2 = grid_gaussian.Nj, jlat;
  while (1)
    {
      jlat = (jlat1 + jlat2) / 2;

      if (jlat2 - jlat1 <= 1)
        break;

      if ((grid_gaussian.indoff_per_lat[jlat1-1] <= itri) && (itri < grid_gaussian.indoff_per_lat[jlat-1]))
        {
          jlat2 = jlat;
	}
      else
      if ((grid_gaussian.indoff_per_lat[jlat-1] <= itri) && (itri < grid_gaussian.indoff_per_lat[jlat2-1]))
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

  int iloen1 = grid_gaussian.pl[jlat1-1];
  int iloen2 = grid_gaussian.pl[jlat2-1];

  auto JDLON = [&iloen1, &iloen2] (int JLON1, int JLON2)
  {
    return (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1));
  };

  int jtri = itri - grid_gaussian.indoff_per_lat[jlat1-1];

  int jlon1 = std::numeric_limits<int>::max (), jlon2 = std::numeric_limits<int>::max ();
  int itriu2 = 0, itrid1 = 0;
  int dtri = 0;

  if (iloen1 == iloen2)
    {
      int jlon = 1 + jtri / 2;
      int jlonn = JNEXT (jlon, iloen1);
      if (jtri % 2)
        {
          jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon -1;
          jglo[1] = grid_gaussian.jglooff[jlat2-1]+jlonn-1; 
          jglo[2] = grid_gaussian.jglooff[jlat1-1]+jlonn-1; 
        }
      else
        {
          jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon -1; 
          jglo[1] = grid_gaussian.jglooff[jlat2-1]+jlon -1;
          jglo[2] = grid_gaussian.jglooff[jlat2-1]+jlonn-1; 
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
      jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon1n-1; 
      jglo[1] = grid_gaussian.jglooff[jlat1-1]+jlon1 -1;
      jglo[2] = grid_gaussian.jglooff[jlat2-1]+jlon2 -1; 
      return;
    }
  else if ((jlon1 == 1) && (jlon2n == 1))
    {
      jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = grid_gaussian.jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = grid_gaussian.jglooff[jlat2-1]+jlon2n-1; 
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
      jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = grid_gaussian.jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = grid_gaussian.jglooff[jlat2-1]+jlon2n-1; 
    }
  else if (idlonn < 0 || ((idlonn == 0) && (idlonc < 0))) 
    {
      jglo[0] = grid_gaussian.jglooff[jlat1-1]+jlon1 -1; 
      jglo[1] = grid_gaussian.jglooff[jlat2-1]+jlon2 -1;
      jglo[2] = grid_gaussian.jglooff[jlat1-1]+jlon1n-1;
    }
  else
    abort ();

}

int glGrib::GeometryGaussian::size () const
{
  return grid_gaussian.jglooff[grid_gaussian.Nj-1] + grid_gaussian.pl[grid_gaussian.Nj-1];
}

void glGrib::GeometryGaussian::setProgramParameters (glGrib::Program * program) const 
{
#include "shaders/include/geometry/buffer_index.h"
#include "shaders/include/geometry/types.h"

  (void)dumm_type;

  if (crds.vertexbuffer != nullptr)
    {
      program->set ("geometry_type", geometry_none);
    }
  else
    {
      program->set ("geometry_type", geometry_gaussian);
      if (! opts.gaussian.fit.on)
        misc_gaussian.ssbo_jlat->bind (GL_SHADER_STORAGE_BUFFER, geometry_gaussian_jlat_idx);
      misc_gaussian.ssbo_jglo->bind (GL_SHADER_STORAGE_BUFFER, geometry_gaussian_jglo_idx);
      misc_gaussian.ssbo_glat->bind (GL_SHADER_STORAGE_BUFFER, geometry_gaussian_glat_idx);
      program->set ("geometry_gaussian_Nj", grid_gaussian.Nj);
      program->set ("geometry_gaussian_omc2", misc_gaussian.omc2);
      program->set ("geometry_gaussian_opc2", misc_gaussian.opc2);
      program->set ("geometry_gaussian_rotated", misc_gaussian.rotated);
      program->set ("geometry_gaussian_rot", misc_gaussian.rot);             
      program->set ("geometry_gaussian_latfit_coeff", misc_gaussian.latfitcoeff);
      program->set ("geometry_gaussian_latfit_degre", misc_gaussian.latfitcoeff.size ()-1);
      program->set ("geometry_gaussian_grid.numberOfPoints", grid.numberOfPoints);
      program->set ("geometry_gaussian_fitlat", opts.gaussian.fit.on);
      program->set ("geometry_gaussian_kind", misc_gaussian.kind);
   }
}

glGrib::GeometryGaussian::GeometryGaussian (int _Nj)
{
  grid_gaussian.Nj = _Nj;

  grid_gaussian.pl.resize (grid_gaussian.Nj);

  for (int jlat = 1; jlat <= grid_gaussian.Nj; jlat++)
    {   
      float lat = pi * (0.5 - static_cast<float> (jlat) / static_cast<float> (grid_gaussian.Nj + 1));
      float coslat = std::cos (lat);
      grid_gaussian.pl[jlat-1] = (2. * grid_gaussian.Nj * coslat);
    }   

  grid.numberOfPoints  = 0;
  for (int i = 0; i < grid_gaussian.Nj; i++)
    grid.numberOfPoints += grid_gaussian.pl[i];

}

glGrib::GeometryGaussian::GeometryGaussian (glGrib::HandlePtr ghp)
{
  codes_handle * h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();

  if (codes_is_defined (h, "stretchingFactor"))
    codes_get_double (h, "stretchingFactor", &misc_gaussian.stretchingFactor);
  if (codes_is_defined (h, "latitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "latitudeOfStretchingPoleInDegrees",
                      &misc_gaussian.latitudeOfStretchingPoleInDegrees);
  if (codes_is_defined (h, "longitudeOfStretchingPoleInDegrees"))
    codes_get_double (h, "longitudeOfStretchingPoleInDegrees", 
                      &misc_gaussian.longitudeOfStretchingPoleInDegrees);
  
  bool do_rot_str = true;

  if (! do_rot_str)
    {
      misc_gaussian.omc2 = 0.0f;
      misc_gaussian.opc2 = 2.0f;
    }
  else
    {
      misc_gaussian.omc2 = 1.0f - 1.0f / (misc_gaussian.stretchingFactor * misc_gaussian.stretchingFactor);
      misc_gaussian.opc2 = 1.0f + 1.0f / (misc_gaussian.stretchingFactor * misc_gaussian.stretchingFactor);
    }
  
  glm::mat4 rot4 = glm::mat4 (1.0f);

  if (do_rot_str) 
  if ((misc_gaussian.latitudeOfStretchingPoleInDegrees != 90.0f && misc_gaussian.longitudeOfStretchingPoleInDegrees != 0.0f)
   || (misc_gaussian.latitudeOfStretchingPoleInDegrees != 0.0f && misc_gaussian.longitudeOfStretchingPoleInDegrees != 0.0f))
    {
      rot4 = glm::rotate (glm::mat4 (1.0f),
                          glm::radians (90.0f-static_cast<float> (misc_gaussian.latitudeOfStretchingPoleInDegrees)), 
                          glm::vec3 (-sinf (glm::radians (misc_gaussian.longitudeOfStretchingPoleInDegrees)),
                                    +cosf (glm::radians (misc_gaussian.longitudeOfStretchingPoleInDegrees)),
                                    0.0f));
      rot4 = rot4 *
            glm::rotate (glm::mat4 (1.0f),
                         glm::radians (180.0f+static_cast<float> (misc_gaussian.longitudeOfStretchingPoleInDegrees)),
                         glm::vec3 (0.0f, 0.0f, 1.0f));
      misc_gaussian.rotated = true;
    }

  misc_gaussian.rot = glm::mat3 (rot4);

  size_t pl_len;
  codes_get_long (h, "Nj", &grid_gaussian.Nj);
  codes_get_size (h, "pl", &pl_len);
  grid_gaussian.pl.resize (pl_len);
  codes_get_long_array (h, "pl", grid_gaussian.pl.data (), &pl_len);

  grid.numberOfPoints  = 0;
  for (int i = 0; i < grid_gaussian.Nj; i++)
    grid.numberOfPoints += grid_gaussian.pl[i];

}

namespace
{
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
}
        
void glGrib::GeometryGaussian::checkTriangleComputation () const
{

  if (0){
  printf (" %8s %8s\n", "jlat", "pl");
  for (int i = 0; i < grid_gaussian.Nj; i++)
    printf (" %8d %8ld\n", i+1, grid_gaussian.pl[i]);
  }

  printf ("----ITRID----\n");

  for (int jlat = 1; jlat <= grid_gaussian.Nj; jlat++)
    {
      int pr = 1;

      for (int jlon = 1; jlon <= grid_gaussian.pl[jlat-1]; jlon++)
        {
          int itrid = computeLowerTriangle (jlat, jlon);
          int jglo = grid_gaussian.jglooff[jlat-1]+jlon-1;

          if (itrid != grid_gaussian.trid[jglo])
	    {
              if (pr) {
              printf (" jlat = %8d\n", jlat);
              printf (" %8s %8s %8s\n", "jlon", "itrid", "itrid");
	      pr = 0;
	      }
              printf (" %8d %8d %8d %1d\n", jlon, itrid, grid_gaussian.trid[jglo], 
                      itrid == grid_gaussian.trid[jglo]);
	    }
     
        }

    }

  printf ("----ITRIU----\n");

  for (int jlat = 1; jlat <= grid_gaussian.Nj; jlat++)
    {
      int pr = 1;

      for (int jlon = 1; jlon <= grid_gaussian.pl[jlat-1]; jlon++)
        {
          int itriu = computeUpperTriangle (jlat, jlon);
          int jglo = grid_gaussian.jglooff[jlat-1]+jlon-1;

          if (itriu != grid_gaussian.triu[jglo])
	    {
              if (pr) {
              printf (" jlat = %8d\n", jlat);
              printf (" %8s %8s %8s\n", "jlon", "itriu", "itriu");
	      pr = 0;
	      }
              printf (" %8d %8d %8d %1d\n", jlon, itriu, grid_gaussian.triu[jglo], 
                      itriu == grid_gaussian.triu[jglo]);
	    }
     
        }

    }

  printf ("-----IND-----\n");

  for (int it = 0; it < static_cast<int> (grid.numberOfTriangles); it++)
    {
      int jglo0[3];
      int jglo1[3];
      for (int i = 0; i < 3; i++)
        jglo0[i] = grid_gaussian.ind[3*it+i];
 
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

  return;
}

void glGrib::GeometryGaussian::setupSSBO ()
{
  // jlat

  if (! opts.gaussian.fit.on)
    {
      misc_gaussian.ssbo_jlat = glGrib::OpenGLBufferPtr<int> (grid.numberOfPoints);
      auto jlat = misc_gaussian.ssbo_jlat->map ();

#pragma omp parallel for
      for (int ilat = 0; ilat < grid_gaussian.Nj; ilat++)
      for (int ilon = 0; ilon < grid_gaussian.pl[ilat]; ilon++)
        jlat[grid_gaussian.jglooff[ilat]+ilon] = ilat;

    }

  // jglooff

  misc_gaussian.ssbo_jglo = glGrib::OpenGLBufferPtr<int> (grid_gaussian.jglooff);

  // Gaussian latitudes
 
  misc_gaussian.ssbo_glat = glGrib::OpenGLBufferPtr<float> (grid_gaussian.Nj);
  auto glat = misc_gaussian.ssbo_glat->map ();

  for (int i = 0; i < grid_gaussian.Nj; i++)
    glat[i] = misc_gaussian.latgauss[i];

}

void glGrib::GeometryGaussian::setupCoordinates ()
{
  crds.vertexbuffer = glGrib::OpenGLBufferPtr<float> (2 * grid.numberOfPoints);
  auto lonlat = crds.vertexbuffer->map ();
  
  int iglooff[grid_gaussian.Nj];
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= grid_gaussian.Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + grid_gaussian.pl[jlat-2];

  float nan = std::numeric_limits<float>::signaling_NaN ();

  // Generation of coordinates
#pragma omp parallel for 
  for (int jlat = 1; jlat <= grid_gaussian.Nj; jlat++)
    {
      float coordy = misc_gaussian.latgauss[jlat-1];
      float lat;
  
      if (misc_gaussian.rotated)
        {
          float sincoordy = std::sin (coordy);
          lat = std::asin ((misc_gaussian.omc2 + sincoordy * misc_gaussian.opc2) / (misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2));
        }
      else
        {
          lat = coordy;
        }
  
      float coslat = nan, sinlat = nan;
  
      if (misc_gaussian.rotated)
        {
          coslat = std::cos (lat); 
          sinlat = std::sin (lat);
        }
  
      int jglo = iglooff[jlat-1];
  
      for (int jlon = 1; jlon <= grid_gaussian.pl[jlat-1]; jlon++, jglo++)
        {
  
          float coordx = twopi * static_cast<float> (jlon-1) / static_cast<float> (grid_gaussian.pl[jlat-1]);
          float lon = coordx;
  
          if (! misc_gaussian.rotated)
            {
              lonlat[2*jglo+0] = lon;
              lonlat[2*jglo+1] = lat;
            }
          else
            {
              float coslon = std::cos (lon); float sinlon = std::sin (lon);
  
              float X = coslon * coslat;
              float Y = sinlon * coslat;
              float Z =          sinlat;
  
              glm::vec3 XYZ = misc_gaussian.rot * glm::vec3 (X, Y, Z);
  
              lonlat[2*jglo+0] = atan2 (XYZ.y, XYZ.x);
              lonlat[2*jglo+1] = std::asin (XYZ.z);
  
            }
  
        }
    }
  
}

namespace 
{

using namespace std;
using namespace glGrib;

#include "shaders/include/geometry/gaussian/kinds.h"

static std::vector<int> jgloff_dum;

class guessJlat
{
public:
#include "shaders/include/geometry/gaussian/guess_jlat.h"
  void geometry_gaussian_miss ()
  {
    miss++;
  }
  int getMissed () const
  {
    return miss;
  }
  void reset ()
  {
    miss = 0;
  }

  std::vector<int> & geometry_gaussian_jglooff = jgloff_dum;
  int   geometry_gaussian_Nj;
  std::vector<float> geometry_gaussian_latfit_coeff;
  int   geometry_gaussian_latfit_degre = -1;
  int   geometry_gaussian_numberOfPoints;
  int   geometry_gaussian_kind;
private:
  int miss = 0;
};

}

void glGrib::GeometryGaussian::setupFitLatitudes ()
{
#include "shaders/include/geometry/gaussian/kinds.h"

  (void)dumm_kind;

  latfit_t latfit[2];

  tryFitLatitudes (geometry_gaussian_kind_lin, &latfit[0]);
  tryFitLatitudes (geometry_gaussian_kind_oct, &latfit[1]);

  std::sort (latfit, latfit+2, [] (const latfit_t & a, const latfit_t & b)
  {
    return a.error < b.error;
  });

  misc_gaussian.kind = latfit[0].kind;
  misc_gaussian.latfitcoeff = latfit[0].coeff;

  if (0)
  std::cout << " kind, error = " << latfit[0].kind 
            << ", " << latfit[0].error << std::endl;
}



void glGrib::GeometryGaussian::tryFitLatitudes (int _kind, latfit_t * latfit)
{
#include "shaders/include/geometry/gaussian/kinds.h"

  (void)dumm_kind;

  latfit->kind  = _kind;

  guessJlat                           gj;
  gj.geometry_gaussian_Nj             = grid_gaussian.Nj;
  gj.geometry_gaussian_numberOfPoints = grid.numberOfPoints;
  gj.geometry_gaussian_kind           = _kind;
  gj.geometry_gaussian_jglooff        = grid_gaussian.jglooff;


  // Prepare data for fitting residual
  std::vector<double> y (grid_gaussian.Nj / 2), x (grid_gaussian.Nj / 2);
  for (int jlat = 0; jlat < grid_gaussian.Nj / 2; jlat++)
    {
      x[jlat] = jlat;
      y[jlat] = double (jlat - gj.geometry_gaussian_guess_jlat0 (grid_gaussian.jglooff[jlat]));
    }

  std::vector<double> coeff;

  const int degree = 3;
  glGrib::FitPolynomial (x, y, degree, coeff);

  gj.geometry_gaussian_latfit_degre      = coeff.size () - 1;
  for (auto c : coeff)
    gj.geometry_gaussian_latfit_coeff.push_back (c);

  for (auto c : coeff)
    latfit->coeff.push_back (c);

  float count_avg = 0, count_max = 0;

  for (int jlat = 0; jlat < grid_gaussian.Nj; jlat++)
  for (int jlon = 0; jlon < grid_gaussian.pl[jlat]; jlon++)
    {
      int jglo = grid_gaussian.jglooff[jlat] + jlon;
      int ilat = gj.geometry_gaussian_guess_jlat (jglo);
      int miss = gj.getMissed ();
      gj.reset ();
      count_avg += miss;
      count_max = std::max (count_max, float (miss));

      if (ilat != jlat)
        {
          printf (" %8d > %8d ; %8d\n", jglo, jlat, ilat);
          throw std::runtime_error ("Latitude fit mismatch");
	}
    }

  latfit->error = count_max;

}

void glGrib::GeometryGaussian::triangulate ()
{

  // Compute number of triangles
  grid.numberOfTriangles = 0;
  for (int jlat = 1; jlat < grid_gaussian.Nj; jlat++)
    grid.numberOfTriangles += grid_gaussian.pl[jlat-1] + grid_gaussian.pl[jlat];
  
  grid_gaussian.indcnt_per_lat = BufferPtr<int> (grid_gaussian.Nj);
  grid_gaussian.indoff_per_lat = BufferPtr<int> (grid_gaussian.Nj);
  // Compute number of triangles per latitude
  grid_gaussian.indoff_per_lat[0] = 0;
  for (int jlat = 1; jlat <= grid_gaussian.Nj; jlat++)
    {
      if (jlat < grid_gaussian.Nj)
        grid_gaussian.indcnt_per_lat[jlat-1] = grid_gaussian.pl[jlat-1] + grid_gaussian.pl[jlat];
      if (jlat > 1)
        grid_gaussian.indoff_per_lat[jlat-1] = grid_gaussian.indoff_per_lat[jlat-2] + grid_gaussian.indcnt_per_lat[jlat-2];
    }


  if (! opts.triangle_strip.on)
    {
      grid_gaussian.ind  = BufferPtr<unsigned int> (3 * grid.numberOfTriangles); 
      grid_gaussian.triu = BufferPtr<int> (grid.numberOfPoints); 
      grid_gaussian.trid = BufferPtr<int> (grid.numberOfPoints); 
      // Generation of triangles
      computeTrigauss (grid_gaussian.Nj, grid_gaussian.pl, grid_gaussian.ind, grid_gaussian.indoff_per_lat, grid_gaussian.indcnt_per_lat, grid_gaussian.triu, grid_gaussian.trid);
      grid.elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (grid_gaussian.ind);
    }
  else
    {
      BufferPtr<int> ind_stripcnt_per_lat (grid_gaussian.Nj+1);
      BufferPtr<int> ind_stripoff_per_lat (grid_gaussian.Nj+1);

      ind_stripoff_per_lat[0] = 0;
      for (int jlat = 1; jlat <= grid_gaussian.Nj+1; jlat++)
        {
          if (jlat < grid_gaussian.Nj) // k1^k2 < (k1 - k2); k1^k2 is the number of possible restarts
            ind_stripcnt_per_lat[jlat-1] = grid_gaussian.pl[jlat-1] + grid_gaussian.pl[jlat] 
                     + 4 * (2 + abs (grid_gaussian.pl[jlat-1] - grid_gaussian.pl[jlat]));
          if (jlat > 1)
            ind_stripoff_per_lat[jlat-1] = ind_stripcnt_per_lat[jlat-2] 
                     + ind_stripoff_per_lat[jlat-2];
        }

  
      grid.ind_strip_size = 0;
      for (int jlat = 1; jlat < grid_gaussian.Nj; jlat++)
        grid.ind_strip_size += ind_stripcnt_per_lat[jlat-1];

      grid.elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (grid.ind_strip_size);
      auto ind_strip = grid.elementbuffer->map ();

      computeTrigaussStrip 
        (grid_gaussian.Nj, grid_gaussian.pl, ind_strip, ind_stripcnt_per_lat, ind_stripoff_per_lat); 

    }

}

void glGrib::GeometryGaussian::setupSubGrid ()
{
  GeometryGaussian * subgridGauss = new GeometryGaussian ();
  subgrid = subgridGauss;

  std::vector<int> i2ip0 (grid_gaussian.Nj);
  std::vector<int> i2ip1 (grid_gaussian.Nj);
  Buffer<unsigned int> l2h;
  
  auto traverse = 
  [this,subgridGauss,&i2ip0,&i2ip1,&l2h] 
    (bool do_point, bool do_lines)
  {
    const auto & pl0 = this->grid_gaussian.pl;
    const auto & Nj0 = this->grid_gaussian.Nj;
  
    int ip0 = 0;
#pragma omp parallel for if (do_point)
    for (int i = 0; i < Nj0; i++)
      {
        int r = i < Nj0 / 2 ? 1 : 0;
        if (i % 2 == r)
          {
            long int rl1 = pl0[i] % 2;
            long int pl1 = (pl0[i] / 2) + rl1;
            if (do_lines)
              {
                i2ip0[i] = ip0;
        	i2ip1[i] = subgridGauss->grid.numberOfPoints;
        	subgridGauss->grid_gaussian.pl.push_back (pl1);
        	subgridGauss->grid_gaussian.Nj++; 
        	subgridGauss->grid.numberOfPoints += subgridGauss->grid_gaussian.pl.back ();
              }
            if (do_point)
              {
                int ip0 = i2ip0[i], ip1 = i2ip1[i];
                for (int j = 0; j < pl0[i]; j++, ip0++)
                  if (j % 2 == 0)
                    l2h[ip1++] = ip0;
                continue;
              }
          }
        ip0 += pl0[i];
      }
  };

  traverse (false, true);

  subgridGauss->triangulate ();

  l2h.allocate (subgridGauss->grid.numberOfPoints);

  traverse (true, false);

  auto bb = subgridGauss->grid.elementbuffer->map ();

  for (size_t i = 0; i < bb.size (); i++)
    if (bb[i] != OpenGL::restart)
      bb[i] = l2h[bb[i]];

  subgridGauss->opts               = opts;
  subgridGauss->misc_gaussian      = misc_gaussian;
  subgridGauss->grid_gaussian.triu = BufferPtr<int> (0);
  subgridGauss->grid_gaussian.trid = BufferPtr<int> (0);
  subgridGauss->crds               = crds;

}


void glGrib::GeometryGaussian::setup 
  (glGrib::HandlePtr ghp, const glGrib::OptionsGeometry & o)
{
  opts = o;

  triangulate ();

  misc_gaussian.latgauss = BufferPtr<double> (grid_gaussian.Nj);

  // Compute Gaussian latitudes
  computeLatgauss (grid_gaussian.Nj, misc_gaussian.latgauss);

  if (! opts.gencoords.on)
    setupCoordinates ();


  grid_gaussian.jglooff.resize (grid_gaussian.Nj + 1);

  grid_gaussian.jglooff[0] = 0;
  for (int jlat = 2; jlat <= grid_gaussian.Nj + 1; jlat++)
    grid_gaussian.jglooff[jlat-1] = grid_gaussian.jglooff[jlat-2] + grid_gaussian.pl[jlat-2];

  if (opts.gencoords.on)
    setupSSBO ();

  if (opts.check.on)
    checkTriangleComputation ();

  if (opts.gaussian.fit.on)
    setupFitLatitudes ();

  setupSubGrid ();
}

void glGrib::GeometryGaussian::latlon2coordxy 
  (float lat, float lon, 
   float & coordx, float & coordy) 
const
{
  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float x, y, z;

  lonlat2xyz (lon, lat, &x, &y, &z);

  glm::vec3 XYZ = glm::inverse (misc_gaussian.rot) * glm::vec3 (x, y, z);

  lon = atan2 (XYZ.y, XYZ.x); 
  lat = std::asin (XYZ.z);

  coordx = lon;
  float sinlat = std::sin (lat);
  coordy = std::asin ((-misc_gaussian.omc2 + sinlat * misc_gaussian.opc2) / (misc_gaussian.opc2 - sinlat * misc_gaussian.omc2));
}

int glGrib::GeometryGaussian::latlon2jlatjlon (float lat, float lon, int & jlat, int & jlon) const
{
  float coordx, coordy;

  latlon2coordxy (lat, lon, coordx, coordy);

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  jlat = round ((0.5 - coordy / pi) * (grid_gaussian.Nj + 1)); // First approximation
  jlat = std::max (1, std::min (jlat, static_cast<int> (grid_gaussian.Nj)));

  float dlat = std::abs (misc_gaussian.latgauss[jlat-1] - coordy);

  while (1)
    {
      bool u = false;
      auto lookat = [&u,&jlat,&dlat,coordy,this] (int jlat1)
      {
        float dlat1 = std::abs (this->misc_gaussian.latgauss[jlat1-1] - coordy);
	if (dlat1 < dlat)
          {
            dlat = dlat1; jlat = jlat1; u = true; 
	  }
	return;
      };
      if (1 < jlat)
        lookat (jlat-1);
      if (jlat < grid_gaussian.Nj)
	lookat (jlat+1);
      if (! u)
        break;
    }

  if (coordx < 0.0f)
    coordx += twopi;
  jlon = round (grid_gaussian.pl[jlat-1] * (coordx / twopi));

  jlat = jlat - 1; // Start at zero

  return 0;
}

int glGrib::GeometryGaussian::latlon2index (float lat, float lon) const
{
  int jlat, jlon;
  latlon2jlatjlon (lat, lon, jlat, jlon);
  return grid_gaussian.jglooff[jlat] + jlon;
}

void glGrib::GeometryGaussian::index2latlon (int jglo, float * lat, float * lon) const
{
  jlonlat_t jlonlat = this->jlonlat (jglo);
  glm::vec2 lonlat = jlonlat2lonlat (jlonlat);
  *lon = lonlat[0];
  *lat = lonlat[1];
}

std::string glGrib::GeometryGaussian::md5 () const
{
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_CTX c;
  MD5_Init (&c);
  
  MD5_Update (&c, &grid_gaussian.Nj, sizeof (grid_gaussian.Nj));
  MD5_Update (&c, grid_gaussian.pl.data (), grid_gaussian.Nj * sizeof (grid_gaussian.pl[0]));
  MD5_Update (&c, &misc_gaussian.stretchingFactor, sizeof (misc_gaussian.stretchingFactor));
  MD5_Update (&c, &misc_gaussian.latitudeOfStretchingPoleInDegrees, sizeof (misc_gaussian.latitudeOfStretchingPoleInDegrees));
  MD5_Update (&c, &misc_gaussian.longitudeOfStretchingPoleInDegrees, sizeof (misc_gaussian.longitudeOfStretchingPoleInDegrees));
  MD5_Final (out, &c);

  return md5string (out);
}


bool glGrib::GeometryGaussian::isEqual (const glGrib::Geometry & other) const
{
  const float epsilon = 1E-4;
  try
    {
      const glGrib::GeometryGaussian & geom = dynamic_cast<const glGrib::GeometryGaussian &>(other);
      return (grid_gaussian.Nj == geom.grid_gaussian.Nj) && (std::abs (misc_gaussian.stretchingFactor - geom.misc_gaussian.stretchingFactor) < epsilon) &&
             (std::abs (misc_gaussian.latitudeOfStretchingPoleInDegrees - geom.misc_gaussian.latitudeOfStretchingPoleInDegrees) < epsilon) &&
	     (std::abs (misc_gaussian.longitudeOfStretchingPoleInDegrees - geom.misc_gaussian.longitudeOfStretchingPoleInDegrees) < epsilon) &&
	     (memcmp (grid_gaussian.pl.data (), geom.grid_gaussian.pl.data (), sizeof (grid_gaussian.pl[0]) * grid_gaussian.Nj) == 0);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void glGrib::GeometryGaussian::applyUVangle (glGrib::BufferPtr<float> & angle) const 
{
  if (misc_gaussian.rotated)
    {
      float coslon0 = std::cos (deg2rad * misc_gaussian.longitudeOfStretchingPoleInDegrees);
      float sinlon0 = std::sin (deg2rad * misc_gaussian.longitudeOfStretchingPoleInDegrees);
      float coslat0 = std::cos (deg2rad * misc_gaussian.latitudeOfStretchingPoleInDegrees);
      float sinlat0 = std::sin (deg2rad * misc_gaussian.latitudeOfStretchingPoleInDegrees);
      glm::vec3 XYZ0 = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);
   
#pragma omp parallel for 
      for (int jlat = 0; jlat < grid_gaussian.Nj; jlat++)
        {
          float coordy = misc_gaussian.latgauss[jlat];
          float sincoordy = std::sin (coordy);
          float lat = std::asin ((misc_gaussian.omc2 + sincoordy * misc_gaussian.opc2) / (misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2));
          float coslat = std::cos (lat); float sinlat = std::sin (lat);
          for (int jlon = 0; jlon < grid_gaussian.pl[jlat]; jlon++)
            {
              int jglo = grid_gaussian.jglooff[jlat] + jlon;

              float coordx = twopi * static_cast<float> (jlon) / static_cast<float> (grid_gaussian.pl[jlat]);
              float lon = coordx;
              float coslon = std::cos (lon); float sinlon = std::sin (lon);
  
              glm::vec3 XYZ = misc_gaussian.rot * glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
  
              glm::vec3 u1 = glm::normalize (glm::cross (XYZ0 - XYZ, XYZ));
              glm::vec3 u0 = glm::normalize (glm::cross (glm::vec3 (0.0f, 0.0f, 1.0f), XYZ));
              glm::vec3 v0 = glm::cross (XYZ, u0);
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

void glGrib::GeometryGaussian::sample (OpenGLBufferPtr<unsigned char> & pp, const unsigned char p0, const int level) const
{
  auto p = pp->map ();
  int lat_stride = static_cast<float> (grid_gaussian.Nj) / static_cast<float> (level);
  if (lat_stride == 0)
    lat_stride = 1;
#pragma omp parallel for 
  for (int jlat = 0; jlat < grid_gaussian.Nj; jlat++)
    {
      float lat = pi * (0.5 - static_cast<float> (jlat+1) / static_cast<float> (grid_gaussian.Nj + 1));
      int lon_stride = lat_stride / std::cos (lat);
      if (lon_stride == 0)
        lon_stride = 1;
      for (int jlon = 0; jlon < grid_gaussian.pl[jlat]; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[grid_gaussian.jglooff[jlat]+jlon] = p0;
    }
}

glGrib::GeometryGaussian::jlonlat_t glGrib::GeometryGaussian::jlonlat (int jglo) const 
{
  int jlat1 = 0, jlat2 = grid_gaussian.Nj, jlat;
  while (jlat2 != jlat1 + 1)
    {
      int jlatm = (jlat1 + jlat2) / 2;
      if ((grid_gaussian.jglooff[jlat1] <= jglo) && (jglo < grid_gaussian.jglooff[jlatm]))
        jlat2 = jlatm;
      else if ((grid_gaussian.jglooff[jlatm] <= jglo) && (jglo < grid_gaussian.jglooff[jlat2]))
        jlat1 = jlatm;
    }
  jlat = 1 + jlat1;
  int jlon = 1 + jglo - grid_gaussian.jglooff[jlat-1];
  return jlonlat_t (jlon, jlat);
}


void glGrib::GeometryGaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], jlonlat_t jlonlat_[3]) const
{
  int jgloABC[3];
  getTriangleVertices (it, jgloABC);
  const int jgloA = jgloABC[0]; 
  const int jgloB = jgloABC[1]; 
  const int jgloC = jgloABC[2]; 
  
  jlonlat_t jlonlatA = jlonlat (jgloA);
  jlonlat_t jlonlatB = jlonlat (jgloB);
  jlonlat_t jlonlatC = jlonlat (jgloC);
  
  int jglo0 = 0, jglo1 = 0, jglo2 = 0;
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
  
  int ntri = grid_gaussian.pl[jlonlat0.jlat-1] + grid_gaussian.pl[jlonlat2.jlat-1];               // Number of triangles on this row
  int lat1 = up ? jlonlat2.jlat : jlonlat0.jlat;
  int otri = lat1 == 1 ? 0 : grid_gaussian.jglooff[lat1] * 2 - grid_gaussian.pl[lat1-1] - grid_gaussian.pl[0];  // Offset of triangles counting
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

int glGrib::GeometryGaussian::getUpperTriangle (int jglo, const jlonlat_t & jlonlat) const
{
  if (grid_gaussian.triu.allocated ())
    return grid_gaussian.triu[jglo];
  else
    return computeUpperTriangle (jlonlat.jlat, jlonlat.jlon);
}

int glGrib::GeometryGaussian::getLowerTriangle (int jglo, const jlonlat_t & jlonlat) const
{
  if (grid_gaussian.trid.allocated ())
    return grid_gaussian.trid[jglo];
  else
    return computeLowerTriangle (jlonlat.jlat, jlonlat.jlon);
}

void glGrib::GeometryGaussian::getTriangleVertices (int it, int jglo[3]) const
{
  if (grid_gaussian.ind)
    {
      jglo[0] = grid_gaussian.ind[3*it+0]; jglo[1] = grid_gaussian.ind[3*it+1]; jglo[2] = grid_gaussian.ind[3*it+2];
    }
  else
    {
      computeTriangleVertices (it, jglo);
    }
}

void glGrib::GeometryGaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
{
  jlonlat_t jlonlat[3];

  getTriangleNeighbours (it, jglo, itri, jlonlat);

  xyz[0] = jlonlat2xyz (jlonlat[0]);
  xyz[1] = jlonlat2xyz (jlonlat[1]);
  xyz[2] = jlonlat2xyz (jlonlat[2]);
}

void glGrib::GeometryGaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec2 merc[3]) const
{
  jlonlat_t jlonlat[3];

  getTriangleNeighbours (it, jglo, itri, jlonlat);

  merc[0] = jlonlat2merc (jlonlat[0]);
  merc[1] = jlonlat2merc (jlonlat[1]);
  merc[2] = jlonlat2merc (jlonlat[2]);
}


bool glGrib::GeometryGaussian::triangleIsEdge (int it) const
{
  int jgloABC[3];
  getTriangleVertices (it, jgloABC);
  const int jgloA = jgloABC[0]; 
  const int jgloB = jgloABC[1]; 
  const int jgloC = jgloABC[2]; 

  if ((jgloA >= grid_gaussian.jglooff[1]) && (jgloB >= grid_gaussian.jglooff[1]) && (jgloC >= grid_gaussian.jglooff[1]) 
   && (jgloA < grid_gaussian.jglooff[grid_gaussian.Nj-1]) && (jgloB < grid_gaussian.jglooff[grid_gaussian.Nj-1]) && (jgloC < grid_gaussian.jglooff[grid_gaussian.Nj-1]))
    return false;
  
  int jglo[3]; 
  int itri[3]; 
  glm::vec3 xyz[3];
  getTriangleNeighbours (it, jglo, itri, xyz);

  return itri[0] < 0;
}


void glGrib::GeometryGaussian::applyNormScale (glGrib::BufferPtr<float> & data) const 
{
  if (! opts.gaussian.apply_norm_scale.on)
    return;
  if (misc_gaussian.stretchingFactor == 1.0f)
    return;
#pragma omp parallel for 
  for (int jlat = 0; jlat < grid_gaussian.Nj; jlat++)
    {
      float coordy = misc_gaussian.latgauss[jlat];
      float sincoordy = std::sin (coordy);
      float N = 1.0f / std::sqrt ((misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2) * (misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2) 
                           / (misc_gaussian.opc2 * misc_gaussian.opc2 - misc_gaussian.omc2 * misc_gaussian.omc2));
      N = 1.0f / N;
      for (int jlon = 0; jlon < grid_gaussian.pl[jlat]; jlon++)
        {
          int jglo = grid_gaussian.jglooff[jlat] + jlon;
          data[jglo] = N * data[jglo];
        }
    }
}

void glGrib::GeometryGaussian::sampleTriangle (glGrib::BufferPtr<unsigned char> & s, const unsigned char s0, const int level) const
{
  int lev = std::max (1, level);
  int itrioff = 0;
  for (int jlat = 1; jlat <= grid_gaussian.Nj-1; jlat++)
    {
      int ntri = grid_gaussian.pl[jlat-1] + grid_gaussian.pl[jlat-1];   // Triangles on this row
      for (int jtri = 1; jtri <= ntri; jtri++)
        if (((jlat - 1) % lev == 0) && ((jtri - 1) % (2 * lev) == 0))
          s[itrioff+jtri-1] = s0;
      itrioff += ntri;
    }
}


int glGrib::GeometryGaussian::getTriangle (float lon, float lat) const
{
  int jlat, jlon;

  float coordx, coordy;

  latlon2coordxy (lat, lon, coordx, coordy);

  if ((coordy > misc_gaussian.latgauss[0]) || (misc_gaussian.latgauss[grid_gaussian.Nj-1] > coordy))
    return -1;

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  jlat = round ((0.5 - coordy / pi) * (grid_gaussian.Nj + 1)); // First approximation
  jlat = std::max (1, std::min (jlat, static_cast<int> (grid_gaussian.Nj)));

  float dlat;

  while ((dlat = coordy - misc_gaussian.latgauss[jlat-1]) > 0.0f)
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
        float dlat1 = coordy - this->misc_gaussian.latgauss[jlat1-1];
	if (std::abs (dlat1) < std::abs (dlat) && (dlat1 <= 0.0f))
          {
            dlat = dlat1; jlat = jlat1; u = true; 
	  }
	return;
      };
      if (1 < jlat)
        lookat (jlat-1);
      if (jlat < grid_gaussian.Nj)
	lookat (jlat+1);
      if (! u)
        break;
    }


  jlat++;

  if (coordx < 0.0f)
    coordx += twopi;
  jlon = static_cast<int> (grid_gaussian.pl[jlat-1] * (coordx / twopi));

  jlat = jlat - 1; // Start at zero

  std::cout << " jlon, jlat = " << jlon << ", " << jlat << std::endl;

  int jglo = grid_gaussian.jglooff[jlat] + jlon;
  std::cout << " it = " << grid_gaussian.triu[jglo] << std::endl;

  return 0;
}

glm::vec2 glGrib::GeometryGaussian::xyz2conformal (const glm::vec3 & xyz) const
{
  float lon = atan2 (xyz.y, xyz.x);
  float lat = std::asin (xyz.z);
  return glm::vec2 (lon, std::log (std::tan (pi / 4.0f + lat / 2.0f)));
}

glm::vec3 glGrib::GeometryGaussian::conformal2xyz (const glm::vec2 & merc) const
{
  float coordx = merc.x;
  float coordy = 2.0f * std::atan (std::exp (merc.y)) - halfpi;
  float sincoordy = std::sin (coordy);

  float lat = std::asin ((misc_gaussian.omc2 + sincoordy * misc_gaussian.opc2) / (misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2));
  float lon = coordx;

  float X, Y, Z;

  lonlat2xyz (lon, lat, &X, &Y, &Z);

  if (! misc_gaussian.rotated)
    return glm::vec3 (X, Y, Z);
  
  return misc_gaussian.rot * glm::vec3 (X, Y, Z);
}

glm::vec2 glGrib::GeometryGaussian::conformal2latlon (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * std::atan (std::exp (merc.y)) - halfpi;
  return glm::vec2 (glm::degrees (lon), glm::degrees (lat));
}

void glGrib::GeometryGaussian::fixPeriodicity (const glm::vec2 & M, glm::vec2 * P, int n) const
{
  // Fix periodicity issue
  for (int i = 0; i < n; i++)
    {
      while (M.x - P[i].x > pi)
        P[i].x += twopi;
      while (P[i].x - M.x > pi)
        P[i].x -= twopi;
    }
}

void glGrib::GeometryGaussian::getPointNeighbours (int jglo, std::vector<int> * neigh) const
{
  neigh->resize (0);
 
  if ((jglo < 0) || (grid.numberOfPoints <= jglo))
    return;

  jlonlat_t jlonlat = this->jlonlat (jglo);

  int jlat = jlonlat.jlat, iloen = grid_gaussian.pl[jlat-1], jlon = jlonlat.jlon; 
  int jlonp = jlon ==     1 ? iloen : jlon - 1;
  int jlonn = jlon == iloen ?     1 : jlon + 1;

  if (jlonlat.jlat < grid_gaussian.Nj)
    {
      // Current point
      int jlat2 = jlonlat.jlat+0, iloen2 = grid_gaussian.pl[jlat2-1], jlon2 = jlonlat.jlon; 
      // Point below
      int jlat1 = jlonlat.jlat+1, iloen1 = grid_gaussian.pl[jlat1-1], jlon1 = 1 + ((jlon2 - 1) * iloen1) / iloen2; 

      if ((jlon1 - 1) * iloen2 == (jlon2 - 1) * iloen1) // Two points are aligned
        {
          int jlon1p = jlon1 ==      1 ? iloen1 : jlon1 - 1;
          int jlon1n = jlon1 == iloen1 ?      1 : jlon1 + 1;
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1p-1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1 -1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1n-1);
        }
      else
        {
          int jlon1n = jlon1 == iloen1 ?      1 : jlon1 + 1;
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1 -1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1n-1);
        }
    }
  
  neigh->push_back (grid_gaussian.jglooff[jlat-1]+jlonn-1);

  if (jlonlat.jlat > 1)
    {
      // Current point
      int jlat2 = jlonlat.jlat+0, iloen2 = grid_gaussian.pl[jlat2-1], jlon2 = jlonlat.jlon; 
      // Point above
      int jlat1 = jlonlat.jlat-1, iloen1 = grid_gaussian.pl[jlat1-1], jlon1 = 1 + ((jlon2 - 1) * iloen1) / iloen2; 

      if ((jlon1 - 1) * iloen2 == (jlon2 - 1) * iloen1) // Two points are aligned
        {
          int jlon1p = jlon1 ==      1 ? iloen1 : jlon1 - 1;
          int jlon1n = jlon1 == iloen1 ?      1 : jlon1 + 1;
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1n-1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1 -1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1p-1);
        }
      else
        {
          int jlon1n = jlon1 == iloen1 ?      1 : jlon1 + 1;
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1n-1);
          neigh->push_back (grid_gaussian.jglooff[jlat1-1]+jlon1 -1);
        }
    }

  neigh->push_back (grid_gaussian.jglooff[jlat-1]+jlonp-1);

  
}

float glGrib::GeometryGaussian::getLocalMeshSize (int jglo) const
{
  float mesh = pi / grid_gaussian.Nj;

  if (misc_gaussian.stretchingFactor == 1.0f)
    return mesh;

  jlonlat_t jlonlat = this->jlonlat (jglo);
  int jlat = jlonlat.jlat-1;

  float coordy = misc_gaussian.latgauss[jlat];
  float sincoordy = std::sin (coordy);
  float N = 1.0f / std::sqrt ((misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2) * (misc_gaussian.opc2 + sincoordy * misc_gaussian.omc2) 
                       / (misc_gaussian.opc2 * misc_gaussian.opc2 - misc_gaussian.omc2 * misc_gaussian.omc2));
  return mesh / N;
}

void glGrib::GeometryGaussian::getView (glGrib::View * view) const
{
  if (! misc_gaussian.rotated)
    return;
  glGrib::OptionsView view_opts = view->getOptions (); 
  view_opts.lon = misc_gaussian.longitudeOfStretchingPoleInDegrees;
  view_opts.lat = misc_gaussian.latitudeOfStretchingPoleInDegrees;
  view->setOptions (view_opts);
}


