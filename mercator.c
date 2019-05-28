#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const double rad2deg = 180.0 / M_PI;
static const double deg2rad = M_PI / 180.0;
static const double a = 6371229.0;

typedef struct
{
  double lat, lon;
} latlon_t;

typedef struct
{
  double x, y;
} xy_t;

typedef struct
{
  latlon_t ref_pt;
  double pole;
  double r_equateur;
  double kl;
} proj_t;

static inline double sign (double a, double b)
{
  return b > 0 ? +fabs (a) : -fabs (a);
}

latlon_t xy_to_latlon (xy_t pt_xy, proj_t p_pj)
{
  latlon_t pt_coord;
  pt_coord.lon = (pt_xy.x / p_pj.r_equateur);
  pt_coord.lat = (M_PI / 2.0) - 2.0 * atan (exp (-(pt_xy.y / p_pj.r_equateur)));
  pt_coord.lon = p_pj.ref_pt.lon + pt_coord.lon;
  return pt_coord;
}

double dist_2ref (latlon_t pt_coord, latlon_t ref_coord)
{
  double z = pt_coord.lon - ref_coord.lon;
  z = z - sign (M_PI, z) * (1.0 + sign (1.0, fabs (z) - M_PI));
  return -z * sign (1.0, z - M_PI);
}

xy_t latlon_to_xy (latlon_t pt_coord, proj_t p_pj)
{
  xy_t pt_xy;
  latlon_t pt_coord2 = pt_coord;
  pt_xy.x = p_pj.r_equateur * dist_2ref (pt_coord, p_pj.ref_pt);
  pt_xy.y = -p_pj.r_equateur * log (tan ((M_PI / 4.0) - (pt_coord2.lat / 2.0)));
  return pt_xy;
}


xy_t xy_new_to_std_origin (latlon_t new_origin_coord, xy_t pt_xy_in_new_origin, proj_t p_pj)
{
  xy_t pt_xy_in_std_origin;
  xy_t n_o_pt_xy = latlon_to_xy (new_origin_coord, p_pj);
  pt_xy_in_std_origin.x = pt_xy_in_new_origin.x+n_o_pt_xy.x;
  pt_xy_in_std_origin.y = pt_xy_in_new_origin.y+n_o_pt_xy.y;
  return pt_xy_in_std_origin;
}


int main (int argc, char * argv[])
{
  xy_t pt_xy;
  proj_t p_pj;
  latlon_t latlon;

  p_pj.ref_pt.lon = deg2rad *  2.0;
  p_pj.ref_pt.lat = deg2rad * 46.7;
  p_pj.pole       = 1.0;

  p_pj.kl = p_pj.pole * sin (p_pj.ref_pt.lat);
  p_pj.r_equateur = a * pow (cos (p_pj.ref_pt.lat), 1.0 - p_pj.kl) * pow (1.0 + p_pj.kl, p_pj.kl) / p_pj.kl;

  pt_xy.x = atof (argv[1]);
  pt_xy.y = atof (argv[2]);

  pt_xy = xy_new_to_std_origin (p_pj.ref_pt, pt_xy, p_pj);

  latlon = xy_to_latlon (pt_xy, p_pj);

  if (latlon.lon < 0)
    latlon.lon += 2 * M_PI;

  printf (" lat = %20.10f, lon = %20.10f\n", rad2deg * latlon.lat, rad2deg * latlon.lon);


  return 0;
}

// cc -g -o mercator.x mercator.c  -lm; ./mercator.x 938750 1001250
