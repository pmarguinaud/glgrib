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
static const double a = 6371229.0;

class latlon_t
{
public:
  latlon_t (double _lon, double _lat) : lon (_lon), lat (_lat) {}
  double lat, lon;
};

class xy_t 
{
public:
  xy_t (double _x, double _y) : x (_x), y (_y) {}
  double x, y;
  xy_t operator+ (const xy_t & p)
  {
    return xy_t (x + p.x, y + p.y);
  }
  xy_t operator- (const xy_t & p)
  {
    return xy_t (x - p.x, y - p.y);
  }
};

class rtheta_t
{
public:
  rtheta_t (double _r, double _theta) : r (_r), theta (_theta) {}
  double r, theta;
};

static inline double sign (double a, double b)
{
  return b > 0 ? +fabs (a) : -fabs (a);
}

static inline double dist_2ref (latlon_t pt_coord, latlon_t ref_coord)
{
  double z = pt_coord.lon - ref_coord.lon;
  z = z - sign (M_PI, z) * (1.0 + sign (1.0, fabs (z) - M_PI));
  return -z * sign (1.0, z - M_PI);
}

class proj_t
{
public:
  proj_t (double lon, double lat, double _pole) : ref_pt (lon, lat), pole (_pole)
  {
    kl = pole * sin (ref_pt.lat);
    r_equateur = a * pow (cos (ref_pt.lat), 1.0 - kl) * pow (1.0 + kl, kl) / kl;
  }
  latlon_t ref_pt;
  double pole;
  double r_equateur;
  double kl;

  latlon_t rtheta_to_latlon (const rtheta_t & pt_rtheta)
  {
    return latlon_t (ref_pt.lon + pt_rtheta.theta / kl,
                     pole * ((M_PI / 2.0) - 2.0 * atan(pow (pt_rtheta.r / r_equateur, 1.0 / kl))));
  }
  rtheta_t xy_to_rtheta (const xy_t & pt_xy)
  {
    double r = sqrt(pt_xy.x * pt_xy.x + pt_xy.y * pt_xy.y);
    double tatng;
    double theta;
  
    if (pt_xy.y == 0.0) 
      tatng = (pt_xy.x == 0.0) ? M_PI : sign (M_PI / 2.0, -pole * pt_xy.x);
    else
      tatng = atan (-pole * (pt_xy.x / pt_xy.y));
    theta = M_PI * sign (1.0, pt_xy.x) * (sign (0.5, pole * pt_xy.y) + 0.5) + tatng;
  
    return rtheta_t (r, theta);
  }
  latlon_t xy_to_latlon (const xy_t & pt_xy)
  {
    return rtheta_to_latlon (xy_to_rtheta (pt_xy));
  }
  rtheta_t latlon_to_rtheta (const latlon_t & pt_coord)
  {
    return rtheta_t 
  	  (r_equateur * pow (tan ((M_PI / 4.0) - ((pole * pt_coord.lat) / 2.0)), kl),
             kl * dist_2ref (pt_coord, ref_pt));
  }
  xy_t rtheta_to_xy (const rtheta_t & pt_rtheta)
  {
    return xy_t (pt_rtheta.r * sin (pt_rtheta.theta), -pole * pt_rtheta.r * cos (pt_rtheta.theta));
  }
  xy_t latlon_to_xy (const latlon_t & pt_coord)
  {
    return rtheta_to_xy (latlon_to_rtheta (pt_coord));
  }

};

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

  proj_t p_pj (deg2rad * LoVInDegrees, deg2rad * LaDInDegrees, projectionCentreFlag == 128 ? -1.0 : +1.0);
  xy_t center_xy = p_pj.latlon_to_xy (p_pj.ref_pt);

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
  proj_t p_pj (deg2rad * LoVInDegrees, deg2rad * LaDInDegrees, projectionCentreFlag == 128 ? -1.0 : +1.0); // Costly

  xy_t xy = p_pj.latlon_to_xy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy_t center_xy = p_pj.latlon_to_xy (p_pj.ref_pt);

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

bool glgrib_geometry_lambert::isEqual (const glgrib_geometry & geom)
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


