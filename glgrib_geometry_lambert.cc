#include "glgrib_geometry_lambert.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glgrib_load.h"
#include <openssl/md5.h>

#include <iostream>
#include <stdexcept>

static const double rad2deg = 180.0 / M_PI;
static const double deg2rad = M_PI / 180.0;

void glgrib_geometry_lambert::gencoords (float * px, float * py) const
{
  throw std::runtime_error (std::string ("glgrib_geometry_lambert::gencoords not implemented"));
}

void glgrib_geometry_lambert::genlatlon (float * plat, float * plon) const
{
  throw std::runtime_error (std::string ("glgrib_geometry_lambert::genlatlon not implemented"));
}

int glgrib_geometry_lambert::size () const
{
  return Nx * Ny;
}

glgrib_geometry_lambert::glgrib_geometry_lambert (codes_handle * h)
{
  codes_get_long (h, "Nx", &Nx);
  codes_get_long (h, "Ny", &Ny);
  codes_get_double (h, "LaDInDegrees", &LaDInDegrees);
  codes_get_double (h, "LoVInDegrees", &LoVInDegrees);
  codes_get_double (h, "DxInMetres", &DxInMetres);
  codes_get_double (h, "DyInMetres", &DyInMetres);
  
}

void glgrib_geometry_lambert::init (codes_handle * h, const float orography)
{
  float * xyz = NULL;
  unsigned int * ind = NULL;

  // Compute number of triangles
  
  numberOfTriangles = 2 * (Nx - 1) * (Ny - 1);
  
  ind = (unsigned int *)malloc (3 * numberOfTriangles * sizeof (unsigned int));
  // Generation of triangles
  for (int j = 0, t = 0; j < Ny-1; j++)
    for (int i = 0; i < Nx-1; i++)
      {
        int ind0 = (j + 0) * Nx + (i + 0); int ind1 = (j + 0) * Nx + (i + 1); 
        int ind2 = (j + 1) * Nx + (i + 0); int ind3 = (j + 1) * Nx + (i + 1); 
        ind[3*t+0] = ind2; ind[3*t+1] = ind0; ind[3*t+2] = ind1; t++;
        ind[3*t+0] = ind2; ind[3*t+1] = ind1; ind[3*t+2] = ind3; t++;
      }

  xyz = (float *)malloc (3 * sizeof (float) * Nx * Ny);
  numberOfPoints  = Nx * Ny;


  const double a = 6371229.0;

  double n = sin (deg2rad * LaDInDegrees);
  double rho0 = a * cos (deg2rad * LaDInDegrees) * pow (tan (deg2rad * LaDInDegrees / 2 + M_PI / 4), n) / n;
  // Generation of coordinates
//#pragma omp parallel for
  for (int j = 0; j < Ny; j++)
    {
      for (int i = 0; i < Nx; i++)
        {
          double X = i * DxInMetres;
          double Y = (Ny / 2 - j) * DyInMetres;
          double rho = sqrt (X * X + (rho0 - Y) * (rho0 - Y));
          double theta = 2 * atan (X / (rho0 + rho - Y));
          double lon = theta / n + LoVInDegrees * deg2rad;
          double lat = M_PI / 2 + atan (pow (rho0 / rho, 1. / n));
          int p = j * Nx + i;
          double x = cos (lon) * cos (lat);
          double y = sin (lon) * cos (lat);
          double z =             sin (lat);
          xyz[3*p+0] = x;
          xyz[3*p+1] = y;
          xyz[3*p+2] = z;
        }
    }
      
  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfTriangles * sizeof (unsigned int), ind);

  free (xyz); xyz = NULL;
  free (ind); ind = NULL;
}

glgrib_geometry_lambert::~glgrib_geometry_lambert ()
{
}

int glgrib_geometry_lambert::latlon2index (float lat, float lon) const
{
#ifdef UNDEF
  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float dl = lon - lon0;
  while (dl < 0.)
    dl += 2 * M_PI;
  while (dl >= 2 * M_PI)
    dl -= 2 * M_PI;

  int i = dl / dlon;
  int j = (lat0 - lat) / dlat;

  if ((i < 0) || (i >= Ni))
    return -1;
  if ((j < 0) || (j >= Nj))
    return -1;

  return j * Ni + Nj;
#endif
  return -1;
}

std::string glgrib_geometry_lambert::md5 () const
{
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_CTX c;
  MD5_Init (&c);

  MD5_Update (&c, &Nx, sizeof (Nx));
  MD5_Update (&c, &Ny, sizeof (Ny));
  MD5_Update (&c, &LaDInDegrees, sizeof (LaDInDegrees));
  MD5_Update (&c, &LoVInDegrees, sizeof (LoVInDegrees));
  MD5_Update (&c, &DxInMetres, sizeof (DxInMetres));
  MD5_Update (&c, &DyInMetres, sizeof (DyInMetres));
  MD5_Final (out, &c);

  return md5string (out);
}

bool glgrib_geometry_lambert::isEqual (const glgrib_geometry & geom)
{
  try
    {
      const glgrib_geometry_lambert & g = dynamic_cast<const glgrib_geometry_lambert &>(geom);
      return (Nx           == g.Nx)
          && (Ny           == g.Ny)
          && (LaDInDegrees == g.LaDInDegrees)
          && (LoVInDegrees == g.LoVInDegrees)
          && (DxInMetres   == g.DxInMetres)
          && (DyInMetres   == g.DyInMetres);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}


