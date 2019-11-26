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
  
  double * zfn = (double *)malloc (sizeof (double) * (kn+1)*(kn+1));
  double * zfnlat = (double *)malloc (sizeof (double) * (kn/2+2));
  double * zmod = (double *)malloc (sizeof (double) * kn);

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
  
  free (zmod);
  free (zfnlat);
  free (zfn);
}

const double glgrib_geometry_gaussian::rad2deg = 180.0 / M_PI;
const double glgrib_geometry_gaussian::deg2rad = M_PI / 180.0;

#define MODULO(x, y) ((x)%(y))
#define JDLON(JLON1, JLON2) (MODULO ((JLON1) - 1, (iloen1)) * (iloen2) - MODULO ((JLON2) - 1, (iloen2)) * (iloen1))
#define JNEXT(JLON, ILOEN) (1 + MODULO ((JLON), (ILOEN)))


#define PRINT(a,b,c) \
  do {                                                          \
    ind[ik++] = (a)-1; ind[ik++] = (b) - 1; ind[ik++] = (c)-1;  \
  } while (0)

static 
void compute_trigauss (const long int Nj, const std::vector<long int> pl, unsigned int * ind, 
                       unsigned int * ind_strip, const int nstripe, const int * indcnt, 
		       const int * ind_stripcnt, int * triu, int * trid)
{
  int iglooff[Nj];
  int indcntoff[nstripe];

  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  indcntoff[0] = 0;
  for (int istripe = 1; istripe < nstripe; istripe++)
    indcntoff[istripe] = indcntoff[istripe-1] + indcnt[istripe-1];

  // No triangles above
  if (triu)
    for (int jlon = 1; jlon <= pl[0]; jlon++)
      triu[iglooff[0]+jlon-1] = -1;

  // No triangles below
  if (trid)
    for (int jlon = 1; jlon <= pl[Nj-1]; jlon++)
      trid[iglooff[Nj-1]+jlon-1] = -1;
  
#pragma omp parallel for 
  for (int istripe = 0; istripe < nstripe; istripe++)
    {
      int jlat1 = 1 + ((istripe + 0) * (Nj-1)) / nstripe;
      int jlat2 = 0 + ((istripe + 1) * (Nj-1)) / nstripe;
      int ik = 3 * indcntoff[istripe];

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

    }

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

void glgrib_geometry_gaussian::setup (glgrib_handle_ptr ghp, const float orography)
{
  codes_handle * h = ghp ? ghp->getCodesHandle () : NULL;
  std::vector<float> xyz;
  const int nstripe = 8;
  int indcnt[nstripe];
  int ind_stripcnt[nstripe];


  bool orog = (orography > 0.0f) && (h != NULL);

  double vmin, vmax, vmis;
  std::vector<double> v;
  if (orog)
    {
      size_t v_len;
      v.resize (numberOfPoints);
      codes_get_double_array (h, "values", v.data (), &v_len);
      codes_get_double (h, "maximum",      &vmax);
      codes_get_double (h, "minimum",      &vmin);
      codes_get_double (h, "missingValue", &vmis);
    }

  // Compute number of triangles
  numberOfTriangles = 0;
  for (int jlat = 1; jlat < Nj; jlat++)
    numberOfTriangles += pl[jlat-1] + pl[jlat];
  
  // Compute number of triangles per stripe
  ind_strip_size = 0;
  for (int istripe = 0; istripe < nstripe; istripe++)
    {
      int jlat1 = 1 + ((istripe + 0) * (Nj-1)) / nstripe;
      int jlat2 = 0 + ((istripe + 1) * (Nj-1)) / nstripe;
      indcnt[istripe] = 0;
      for (int jlat = jlat1; jlat <= jlat2; jlat++)
        indcnt[istripe] += pl[jlat-1] + pl[jlat];
      ind_stripcnt[istripe] = 0;
      for (int jlat = jlat1; jlat <= jlat2; jlat++)
        ind_stripcnt[istripe] += pl[jlat-1] + pl[jlat]
                               + 4 * (2 + abs (pl[jlat-1] - pl[jlat]));
      ind_strip_size += ind_stripcnt[istripe];
    }

  ind = (unsigned int *)malloc (3 * numberOfTriangles * sizeof (unsigned int));
  unsigned int * ind_strip = (unsigned int *)malloc (ind_strip_size * sizeof (unsigned int));
  triu = (int *)malloc (numberOfPoints * sizeof (int));
  trid = (int *)malloc (numberOfPoints * sizeof (int));
  // Generation of triangles
  compute_trigauss (Nj, pl, ind, ind_strip, nstripe, indcnt, ind_stripcnt, triu, trid);

  latgauss = (double *)malloc (Nj * sizeof (double));
  // Compute Gaussian latitudes
  compute_latgauss (Nj, latgauss);
      
  xyz.resize (3 * numberOfPoints);

  int iglooff[Nj];
  iglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj; jlat++)
    iglooff[jlat-1] = iglooff[jlat-2] + pl[jlat-2];

  // OpenMP generation of coordinates
#pragma omp parallel for 
  for (int jlat = 1; jlat <= Nj; jlat++)
    {
      float coordy = latgauss[jlat-1];
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
 
          if (orog)
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


  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (xyz[0]), xyz.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfTriangles * sizeof (ind[0]), ind);

  jglooff.resize (Nj + 1);

  jglooff[0] = 0;
  for (int jlat = 2; jlat <= Nj + 1; jlat++)
    jglooff[jlat-1] = jglooff[jlat-2] + pl[jlat-2];
}

glgrib_geometry_gaussian::~glgrib_geometry_gaussian ()
{
  if (ind)
    free (ind);
  if (triu)
    free (triu);
  if (trid)
    free (trid);
  if (latgauss)
    free (latgauss);
  ind = NULL;
  triu = NULL;
  trid = NULL;
  latgauss = NULL;
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


bool glgrib_geometry_gaussian::isEqual (const glgrib_geometry & geom) const
{
  const float epsilon = 1E-4;
  try
    {
      const glgrib_geometry_gaussian & g = dynamic_cast<const glgrib_geometry_gaussian &>(geom);
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


void glgrib_geometry_gaussian::getTriangleVertices (int it, int jglo[3]) const
{
  jglo[0] = ind[3*it+0];
  jglo[1] = ind[3*it+1];
  jglo[2] = ind[3*it+2];
}

void glgrib_geometry_gaussian::getTriangleNeighbours (int it, int jglo[3], int itri[3], jlonlat_t jlonlat_[3]) const
{
  int jgloA = ind[3*it+0]; 
  int jgloB = ind[3*it+1]; 
  int jgloC = ind[3*it+2]; 
  
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
  int Vtri = up ? trid[jglo0] : triu[jglo0];                          // Rank of triangle above of under segment 01

  int itr01 = Vtri, itr12 = Rtri, itr20 = Ltri;
  
  jglo[0] = jglo0; jglo[1] = jglo1; jglo[2] = jglo2;
  itri[0] = itr01; itri[1] = itr12; itri[2] = itr20;

  jlonlat_[0] = jlonlat0;
  jlonlat_[1] = jlonlat1;
  jlonlat_[2] = jlonlat2;
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
  int jgloA = ind[3*it+0]; 
  int jgloB = ind[3*it+1]; 
  int jgloC = ind[3*it+2]; 

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
