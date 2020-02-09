

float sign2 (float a, float b)
{
  return b > 0 ? +abs (a) : -abs (a);
}

struct latlon_t
{
  float lon, lat;
};

struct latlon_j_t
{
  latlon_t V, A, B;
};

struct xy_t
{
  float x, y;
};

struct xy_j_t
{
  xy_t V, A, B;
};

struct rtheta_t
{
  float r, theta;
};

struct rtheta_j_t
{
  rtheta_t V, A, B;
};

float dist_2ref (latlon_t pt_coord, latlon_t ref_coord)
{
  float z = pt_coord.lon - ref_coord.lon;
  z = z - sign2 (pi, z) * (1.0 + sign2 (1.0, abs (z) - pi));
  return -z * sign2 (1.0, z - pi);
}
  
struct proj_t
{
  latlon_t ref_pt;
  float pole;
  float r_equateur;
  float kl;
};

uniform float geometry_lambert_p_pj_ref_pt_lon;
uniform float geometry_lambert_p_pj_ref_pt_lat;
uniform float geometry_lambert_p_pj_pole;
uniform float geometry_lambert_p_pj_r_equateur;
uniform float geometry_lambert_p_pj_kl;
uniform int   geometry_lambert_Nx;
uniform int   geometry_lambert_Ny;
uniform int   geometry_lambert_Nux;
uniform int   geometry_lambert_Nuy;
uniform float geometry_lambert_DxInMetres;
uniform float geometry_lambert_DyInMetres;
uniform float geometry_lambert_center_xy_x;
uniform float geometry_lambert_center_xy_y;

proj_t get_proj_t ()
{
  proj_t proj = proj_t (latlon_t (geometry_lambert_p_pj_ref_pt_lon,
                                  geometry_lambert_p_pj_ref_pt_lat),
                        geometry_lambert_p_pj_pole,
                        geometry_lambert_p_pj_r_equateur,
                        geometry_lambert_p_pj_kl);
  return proj;
}

rtheta_t xy_to_rtheta (proj_t proj, xy_t pt_xy)
{
  float r = sqrt (pt_xy.x * pt_xy.x + pt_xy.y * pt_xy.y);
  float tatng;
  float theta;

  if (pt_xy.y == 0.0) 
    tatng = (pt_xy.x == 0.0) ? pi : sign2 (pi / 2.0, -proj.pole * pt_xy.x);
  else
    tatng = atan (-proj.pole * (pt_xy.x / pt_xy.y));
  theta = pi * sign2 (1.0, pt_xy.x) * (sign2 (0.5, proj.pole * pt_xy.y) + 0.5) + tatng;

  return rtheta_t (r, theta);
}

latlon_t rtheta_to_latlon (proj_t proj, rtheta_t pt_rtheta) 
{
  return latlon_t (proj.ref_pt.lon + pt_rtheta.theta / proj.kl,
                   proj.pole * ((pi / 2.0) - 2.0 * atan (pow (pt_rtheta.r / proj.r_equateur, 1.0 / proj.kl))));
}

latlon_t xy_to_latlon (proj_t proj, xy_t pt_xy) 
{
  return rtheta_to_latlon (proj, xy_to_rtheta (proj, pt_xy));
}

vec2 getLambertVertexLonlat (int jglo)
{
  int i = jglo % geometry_lambert_Nx;
  int j = jglo / geometry_lambert_Nx;

  proj_t proj = get_proj_t ();

  xy_t pt_xy = xy_t ((i - geometry_lambert_Nux / 2) * geometry_lambert_DxInMetres, 
                     (j - geometry_lambert_Nuy / 2) * geometry_lambert_DyInMetres);
  
  pt_xy.x = pt_xy.x + geometry_lambert_center_xy_x;
  pt_xy.y = pt_xy.y + geometry_lambert_center_xy_y;

  latlon_t latlon = xy_to_latlon (proj, pt_xy);

  return vec2 (latlon.lon, latlon.lat);
}  



