#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <utility>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>


const double rad2deg = 180.0 / M_PI;
const double deg2rad = M_PI / 180.0;
const double a = 6371229.0;

  
class latlon_t
{
public:
  latlon_t () {}
  latlon_t (double _lon, double _lat) : lon (_lon), lat (_lat) {}
  double lat = 0.0, lon = 0.0;
  latlon_t operator+ (const latlon_t & p)
  {
    return latlon_t (lon + p.lon, lat + p.lat);
  }
  latlon_t operator- (const latlon_t & p)
  {
    return latlon_t (lon - p.lon, lat - p.lat);
  }
};

class latlon_j_t
{
public:
  latlon_j_t (const latlon_t & _V, const latlon_t & _A, const latlon_t & _B) 
    : V (_V), A (_A), B (_B) {}
  latlon_t V, A, B;
};

class xy_t 
{
public:
  xy_t () {}
  xy_t (double _x, double _y) : x (_x), y (_y) {}
  double x = 0.0, y = 0.0;
  xy_t operator+ (const xy_t & p)
  {
    return xy_t (x + p.x, y + p.y);
  }
  xy_t operator- (const xy_t & p)
  {
    return xy_t (x - p.x, y - p.y);
  }
};

class xy_j_t
{
public:
  xy_j_t (const xy_t & _V, const xy_t & _A, const xy_t & _B) 
    : V (_V), A (_A), B (_B) {}
  xy_t V, A, B;
};

class rtheta_t
{
public:
  rtheta_t (double _r, double _theta) : r (_r), theta (_theta) {}
  double r, theta;
};

class rtheta_j_t
{
public:
  rtheta_j_t (const rtheta_t & _V, const rtheta_t & _A, const rtheta_t & _B) 
    : V (_V), A (_A), B (_B) {}
  rtheta_t V, A, B;
};

static inline double sign (const double & a, const double & b)
{
  return b > 0 ? +fabs (a) : -fabs (a);
}

static inline double dist_2ref (const latlon_t & pt_coord, const latlon_t & ref_coord)
{
  double z = pt_coord.lon - ref_coord.lon;
  z = z - sign (M_PI, z) * (1.0 + sign (1.0, fabs (z) - M_PI));
  return -z * sign (1.0, z - M_PI);
}

class proj_t
{
public:
  proj_t () {}
  proj_t (double lon, double lat, double _pole) : ref_pt (lon, lat), pole (_pole)
  {
    kl = pole * sin (ref_pt.lat);
    r_equateur = a * pow (cos (ref_pt.lat), 1.0 - kl) * pow (1.0 + kl, kl) / kl;
  }
  latlon_t ref_pt;
  double pole;
  double r_equateur;
  double kl;

  latlon_j_t rtheta_to_latlon (const rtheta_j_t & pt_rtheta) const 
  {
    double lon = ref_pt.lon + pt_rtheta.V.theta / kl;
    double dlon_dA = pt_rtheta.A.theta / kl;
    double dlon_dB = pt_rtheta.B.theta / kl;

    double lat = pole * ((M_PI / 2.0) - 2.0 * atan (pow (pt_rtheta.V.r / r_equateur, 1.0 / kl)));

    double lat1 = pole * ((M_PI / 2.0) - 2.0 * atan (pow ((pt_rtheta.V.r-1) / r_equateur, 1.0 / kl)));

    double dlat_dA = pole * (-2.0 * pow (pt_rtheta.V.r, 1.0 / kl - 1.0) * pt_rtheta.A.r) /
	          (kl * pow (r_equateur, 1.0 / kl) * (1.0 + pow (pt_rtheta.V.r / r_equateur, 2.0 / kl)));
    double dlat_dB = pole * (-2.0 * pow (pt_rtheta.V.r, 1.0 / kl - 1.0) * pt_rtheta.B.r) /
	          (kl * pow (r_equateur, 1.0 / kl) * (1.0 + pow (pt_rtheta.V.r / r_equateur, 2.0 / kl)));

    return latlon_j_t (latlon_t (lon, lat), latlon_t (dlon_dA, dlat_dA), latlon_t (dlon_dB, dlat_dB));
  }
  latlon_t rtheta_to_latlon (const rtheta_t & pt_rtheta) const 
  {
    return latlon_t (ref_pt.lon + pt_rtheta.theta / kl,
                     pole * ((M_PI / 2.0) - 2.0 * atan(pow (pt_rtheta.r / r_equateur, 1.0 / kl))));
  }
  rtheta_j_t xy_to_rtheta (const xy_j_t & pt_xy) const 
  {
    double r2 = pt_xy.V.x * pt_xy.V.x + pt_xy.V.y * pt_xy.V.y;
    double r = sqrt (r2);
    double dr_dA = (pt_xy.V.x * pt_xy.A.x + pt_xy.V.y * pt_xy.A.y) / r;
    double dr_dB = (pt_xy.V.x * pt_xy.B.x + pt_xy.V.y * pt_xy.B.y) / r;
    double tatng, dtatng_dA, dtatng_dB;
    double theta, dtheta_dA, dtheta_dB;
  

    if (pt_xy.V.y == 0.0) 
      tatng = (pt_xy.V.x == 0.0) ? M_PI : sign (M_PI / 2.0, -pole * pt_xy.V.x);
    else
      tatng = atan (-pole * (pt_xy.V.x / pt_xy.V.y));
    if (r2 > 0.0)
      {
        dtatng_dA = -pole * (pt_xy.V.y * pt_xy.A.x - pt_xy.V.x * pt_xy.A.y) / r2;
        dtatng_dB = -pole * (pt_xy.V.y * pt_xy.B.x - pt_xy.V.x * pt_xy.B.y) / r2;
      }
    else
      {
        dtatng_dA = 0.0;
        dtatng_dB = 0.0;
      }

    theta = M_PI * sign (1.0, pt_xy.V.x) * (sign (0.5, pole * pt_xy.V.y) + 0.5) + tatng;
    dtheta_dA = dtatng_dA;
    dtheta_dB = dtatng_dB;

    return rtheta_j_t (rtheta_t (r, theta), rtheta_t (dr_dA, dtheta_dA), rtheta_t (dr_dB, dtheta_dB));
  }
  rtheta_t xy_to_rtheta (const xy_t & pt_xy) const 
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
  latlon_t xy_to_latlon (const xy_t & pt_xy) const 
  {
    return rtheta_to_latlon (xy_to_rtheta (pt_xy));
  }
  latlon_j_t xy_to_latlon (const xy_j_t & pt_xy) const
  {
    return rtheta_to_latlon (xy_to_rtheta (pt_xy));
  }
  rtheta_t latlon_to_rtheta (const latlon_t & pt_coord) const 
  {
    return rtheta_t 
  	  (r_equateur * pow (tan ((M_PI / 4.0) - ((pole * pt_coord.lat) / 2.0)), kl),
             kl * dist_2ref (pt_coord, ref_pt));
  }
  xy_t rtheta_to_xy (const rtheta_t & pt_rtheta) const 
  {
    return xy_t (pt_rtheta.r * sin (pt_rtheta.theta), -pole * pt_rtheta.r * cos (pt_rtheta.theta));
  }
  xy_t latlon_to_xy (const latlon_t & pt_coord) const 
  {
    return rtheta_to_xy (latlon_to_rtheta (pt_coord));
  }

};


int main (int argc, char * argv[])
{
  double  x = atof (argv[1]);
  double  y = atof (argv[2]);

  proj_t p_pj (deg2rad * 2.0, deg2rad * 46.7, +1.0f);
  xy_t center_xy = p_pj.latlon_to_xy (p_pj.ref_pt);

  xy_t xy (x, y);
  xy = xy + center_xy;
  
  xy_t xy1 = xy + xy_t (1.0, 0.0);

  latlon_t latlon = p_pj.xy_to_latlon (xy);
  latlon_t latlonx = p_pj.xy_to_latlon (xy + xy_t (1.0, 0.0));
  latlon_t latlony = p_pj.xy_to_latlon (xy + xy_t (0.0, 1.0));

  printf (" %30.20f %30.20f\n", a * latlon.lon, a * latlon.lat);
  printf (" %30.20f %30.20f\n", a * latlonx.lon, a * latlonx.lat);
  printf (" %30.20f %30.20f\n", a * latlony.lon, a * latlony.lat);
  
  xy_j_t xy_j (xy, xy_t (1.0, 0.0), xy_t (0.0, 1.0));

  latlon_j_t latlon_j = p_pj.xy_to_latlon (xy_j);

  printf (" %30.20f %30.20f\n", a * cos (latlon.lat) * latlon_j.A.lon, a * cos (latlon.lat) * (latlonx.lon - latlon.lon));
  printf (" %30.20f %30.20f\n", a *                    latlon_j.A.lat, a *                    (latlonx.lat - latlon.lat));
  printf (" %30.20f %30.20f\n", a * cos (latlon.lat) * latlon_j.B.lon, a * cos (latlon.lat) * (latlony.lon - latlon.lon));
  printf (" %30.20f %30.20f\n", a *                    latlon_j.B.lat, a *                    (latlony.lat - latlon.lat));


// | lon |   | latlon_j.A.lon latlon_j.B.lon |   | X |
// |     | x |                               | x |   |
// | lat |   | latlon_j.A.lat latlon_j.B.lat |   | Y |
//
//

  glm::vec2 X (a * cos (latlon.lat) * latlon_j.A.lon, a * latlon_j.A.lat);
  glm::vec2 Y (a * cos (latlon.lat) * latlon_j.B.lon, a * latlon_j.B.lat);

  std::cout << X.x << " " << X.y << std::endl;
  std::cout << Y.x << " " << Y.y << std::endl;

  std::cout << glm::dot (X, X) << std::endl;
  std::cout << glm::dot (Y, Y) << std::endl;
  std::cout << glm::dot (X, Y) << std::endl;




  return 0;
}

