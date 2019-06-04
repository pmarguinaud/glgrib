#include "glgrib_geometry_lambert.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glgrib_load.h"
#include <openssl/md5.h>

#include <iostream>
#include <stdexcept>

const double glgrib_geometry_lambert::rad2deg = 180.0 / M_PI;
const double glgrib_geometry_lambert::deg2rad = M_PI / 180.0;
const double glgrib_geometry_lambert::a = 6371229.0;

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
  codes_get_long (h, "Nux", &Nux);
  codes_get_long (h, "Nuy", &Nuy);
  codes_get_long (h, "projectionCentreFlag", &projectionCentreFlag);
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

  p_pj = proj_t (deg2rad * LoVInDegrees, deg2rad * LaDInDegrees, projectionCentreFlag == 128 ? -1.0 : +1.0);
  center_xy = p_pj.latlon_to_xy (p_pj.ref_pt);

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < Ny; j++)
    for (int i = 0; i < Nx; i++)
      {
        xy_t pt_xy ((i - Nux / 2) * DxInMetres, (j - Nuy / 2) * DyInMetres);
        pt_xy = pt_xy + center_xy;
        latlon_t latlon = p_pj.xy_to_latlon (pt_xy);

        int p = j * Nx + i;
        xyz[3*p+0] = cos (latlon.lon) * cos (latlon.lat);
        xyz[3*p+1] = sin (latlon.lon) * cos (latlon.lat);
        xyz[3*p+2] =                    sin (latlon.lat);
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
  xy_t xy = p_pj.latlon_to_xy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy = xy - center_xy;

  int i = xy.x / DxInMetres + Nux / 2;
  int j = xy.y / DyInMetres + Nuy / 2;

  if ((i < 0) || (i >= Nx))
    return -1;
  if ((j < 0) || (j >= Ny))
    return -1;

  return j * Nx + i;
}

std::string glgrib_geometry_lambert::md5 () const
{
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_CTX c;
  MD5_Init (&c);

  MD5_Update (&c, &Nx, sizeof (Nx));
  MD5_Update (&c, &Ny, sizeof (Ny));
  MD5_Update (&c, &Nux, sizeof (Nux));
  MD5_Update (&c, &Nuy, sizeof (Nuy));
  MD5_Update (&c, &projectionCentreFlag, sizeof (projectionCentreFlag));
  MD5_Update (&c, &LaDInDegrees, sizeof (LaDInDegrees));
  MD5_Update (&c, &LoVInDegrees, sizeof (LoVInDegrees));
  MD5_Update (&c, &DxInMetres, sizeof (DxInMetres));
  MD5_Update (&c, &DyInMetres, sizeof (DyInMetres));
  MD5_Final (out, &c);

  return md5string (out);
}

bool glgrib_geometry_lambert::isEqual (const glgrib_geometry & geom) const
{
  try
    {
      const glgrib_geometry_lambert & g = dynamic_cast<const glgrib_geometry_lambert &>(geom);
      return (Nx                    == g.Nx)
          && (Ny                    == g.Ny)
          && (Nux                   == g.Nux)
          && (Nuy                   == g.Nuy)
          && (projectionCentreFlag  == g.projectionCentreFlag)
          && (LaDInDegrees          == g.LaDInDegrees)
          && (LoVInDegrees          == g.LoVInDegrees)
          && (DxInMetres            == g.DxInMetres)
          && (DyInMetres            == g.DyInMetres);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void glgrib_geometry_lambert::sample (unsigned char * p, const unsigned char p0, const int level) const
{
  xy_t pt_sw ((-Nux / 2) * DxInMetres, (-Nuy / 2) * DyInMetres);
  xy_t pt_ne ((+Nux / 2) * DxInMetres, (+Nuy / 2) * DyInMetres);
  pt_sw = pt_sw + center_xy;
  pt_ne = pt_ne + center_xy;
  latlon_t latlon_sw = p_pj.xy_to_latlon (pt_sw);
  latlon_t latlon_ne = p_pj.xy_to_latlon (pt_ne);
  
  float Dlat = latlon_ne.lat - latlon_sw.lat;
  float Dlon = latlon_ne.lon - latlon_sw.lon;
  float lat = (latlon_ne.lat + latlon_sw.lat) / 2.0f;
  
  int lat_stride = (Ny * M_PI) / (level * Dlat);
  if (lat_stride == 0)
    lat_stride = 1;

  for (int jlat = 0; jlat < Ny; jlat++)
    {
      int lon_stride = (lat_stride * Dlat) / (Dlon * cos (lat));
      if (lon_stride == 0)
        lon_stride = 1;
      for (int jlon = 0; jlon < Nx; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jlat*Nx+jlon] = p0;
    }
}

float glgrib_geometry_lambert::resolution (int level) const 
{
  if (level == 0)
    level == Ny;
  xy_t pt_sw ((-Nux / 2) * DxInMetres, (-Nuy / 2) * DyInMetres);
  xy_t pt_ne ((+Nux / 2) * DxInMetres, (+Nuy / 2) * DyInMetres);
  pt_sw = pt_sw + center_xy;
  pt_ne = pt_ne + center_xy;
  latlon_t latlon_sw = p_pj.xy_to_latlon (pt_sw);
  latlon_t latlon_ne = p_pj.xy_to_latlon (pt_ne);
  float Dlat = latlon_ne.lat - latlon_sw.lat;
  return Dlat / level;
}

