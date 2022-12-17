#include "glGrib/GeometryLambert.h"
#include "glGrib/MD5.h"

#include <cmath>

#include <iostream>
#include <stdexcept>

namespace glGrib
{

const double GeometryLambert::a = 6371229.0;

void GeometryLambert::setupCoordinates ()
{
  crds.vertexbuffer = OpenGLBufferPtr<float> (2 * grid.numberOfPoints);

  auto lonlat = crds.vertexbuffer->map ();

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < grid_lambert.Ny; j++)
    for (int i = 0; i < grid_lambert.Nx; i++)
      {
        xy_t pt_xy ((i - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
                    (j - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
        pt_xy = pt_xy + misc_lambert.center_xy;

        latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);

        int p = j * grid_lambert.Nx + i;
	lonlat[2*p+0] = latlon.lon;
	lonlat[2*p+1] = latlon.lat;
      }
}

void GeometryLambert::setProgramParameters (Program * program) const 
{
#include "shaders/include/geometry/types.h"
 
  (void)dumm_type;

  if (crds.vertexbuffer != nullptr)
    {
      program->set ("geometry_type", geometry_none);
    }
  else
    {
      program->set ("geometry_type", geometry_lambert);
      program->set ("geometry_lambert_p_pj_ref_pt_lon",   misc_lambert.p_pj.ref_pt.lon );
      program->set ("geometry_lambert_p_pj_ref_pt_lat",   misc_lambert.p_pj.ref_pt.lat );
      program->set ("geometry_lambert_p_pj_pole",         misc_lambert.p_pj.pole       );
      program->set ("geometry_lambert_p_pj_r_equateur",   misc_lambert.p_pj.r_equateur );
      program->set ("geometry_lambert_p_pj_kl",           misc_lambert.p_pj.kl         );
      program->set ("geometry_lambert_Nx",                grid_lambert.Nx              );
      program->set ("geometry_lambert_Ny",                grid_lambert.Ny              );
      program->set ("geometry_lambert_Nux",               misc_lambert.Nux             );
      program->set ("geometry_lambert_Nuy",               misc_lambert.Nuy             );
      program->set ("geometry_lambert_DxInMetres",        misc_lambert.DxInMetres      );
      program->set ("geometry_lambert_DyInMetres",        misc_lambert.DyInMetres      );
      program->set ("geometry_lambert_center_xy_x",       misc_lambert.center_xy.x     );
      program->set ("geometry_lambert_center_xy_y",       misc_lambert.center_xy.y     );
    }
}

int GeometryLambert::size () const
{
  return grid_lambert.Nx * grid_lambert.Ny;
}

GeometryLambert::GeometryLambert (HandlePtr ghp)
{
  codes_handle * h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();
  codes_get_long (h, "Nx", &grid_lambert.Nx);
  codes_get_long (h, "Ny", &grid_lambert.Ny);
  codes_get_long (h, "Nux", &misc_lambert.Nux);
  codes_get_long (h, "Nuy", &misc_lambert.Nuy);
  codes_get_long (h, "projectionCentreFlag", &misc_lambert.projectionCentreFlag);
  codes_get_double (h, "LaDInDegrees", &misc_lambert.LaDInDegrees);
  codes_get_double (h, "LoVInDegrees", &misc_lambert.LoVInDegrees);
  codes_get_double (h, "DxInMetres", &misc_lambert.DxInMetres);
  codes_get_double (h, "DyInMetres", &misc_lambert.DyInMetres);
  
}

void GeometryLambert::setupFrame ()
{
  frame.numberOfPoints = 2 * (grid_lambert.Nx + grid_lambert.Ny - 2);
  frame.vertexbuffer = OpenGLBufferPtr<float> (3 * (frame.numberOfPoints + 2));
  
  auto lonlat = frame.vertexbuffer->map ();
  
  int p = 0;
  
  auto push = [&lonlat, &p, this] (int i, int j, int latcst)
  {
    xy_t pt_xy ((i - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
                (j - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
    pt_xy = pt_xy + misc_lambert.center_xy;
  
    latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);
  
    lonlat[3*p+0] = latlon.lon;
    lonlat[3*p+1] = latlon.lat;
    lonlat[3*p+2] = latcst == 0 ? 1.0f : 0.0f;
    p++;
  };
   
  for (int j = 0; j < grid_lambert.Ny-1; j++)
    push (0, j, 1);
  
  for (int i = 0; i < grid_lambert.Nx-1; i++)
    push (i, grid_lambert.Ny-1, 0);
  
  for (int j = grid_lambert.Ny-1; j >= 1; j--)
    push (grid_lambert.Nx-1, j, 1);
  
  for (int i = grid_lambert.Nx-1; i >= 1; i--)
    push (i, 0, 0);
  
  for (int j = 0; j < 2; j++)
    push (0, j, 1);
  
}


void GeometryLambert::setup (HandlePtr ghp, const OptionsGeometry & o)
{
  opts = o;

  // Compute number of triangles
  
  grid.numberOfTriangles = 2 * (grid_lambert.Nx - 1) * (grid_lambert.Ny - 1);
  grid.numberOfPoints  = grid_lambert.Nx * grid_lambert.Ny;
  
  // Generation of triangles
  
  if (! opts.triangle_strip.on)
    {
      grid.elementbuffer = OpenGLBufferPtr<unsigned int> (3 * grid.numberOfTriangles);
      auto ind = grid.elementbuffer->map ();

      for (int j = 0, t = 0; j < grid_lambert.Ny-1; j++)
        for (int i = 0; i < grid_lambert.Nx-1; i++)
          {
            int ind0 = (j + 0) * grid_lambert.Nx + (i + 0); 
            int ind1 = (j + 0) * grid_lambert.Nx + (i + 1); 
            int ind2 = (j + 1) * grid_lambert.Nx + (i + 0); 
            int ind3 = (j + 1) * grid_lambert.Nx + (i + 1); 
            ind[3*t+0] = ind1; ind[3*t+1] = ind3; ind[3*t+2] = ind2; t++;
            ind[3*t+0] = ind0; ind[3*t+1] = ind1; ind[3*t+2] = ind2; t++;
          }
    }
  else
    {
      grid.ind_strip_size = (2 * grid_lambert.Nx + 5) * (grid_lambert.Ny - 1);
      grid.elementbuffer = OpenGLBufferPtr<unsigned int> (grid.ind_strip_size);
  
      auto ind_strip = grid.elementbuffer->map ();

      auto ind0 = [=] (int i, int j) { return (j + 0) * grid_lambert.Nx + (i + 0); };
      auto ind2 = [=] (int i, int j) { return (j + 1) * grid_lambert.Nx + (i + 0); };

      int t = 0;
      for (int j = 0; j < grid_lambert.Ny-1; j++)
        {
          // Control triangle culling : first triangle cannot be part of the strip
          ind_strip[t++] = ind2 (0, j); 
          ind_strip[t++] = ind0 (0, j);
          ind_strip[t++] = ind0 (1, j);
          ind_strip[t++] = OpenGL::restart;
          // Following triangles
          for (int i = 0; i < grid_lambert.Nx-1; i++)
            {
              ind_strip[t++] = ind2 (i, j); ind_strip[t++] = ind0 (i+1, j);
            }
          // Last triangle
          ind_strip[t++] = ind2 (grid_lambert.Nx-1, j);
	  ind_strip[t++] = OpenGL::restart;
	}

      if (t >= static_cast<int> (grid.ind_strip_size))
        abort ();

      for (; t < static_cast<int> (grid.ind_strip_size); t++)
	ind_strip[t] = OpenGL::restart;
      
    }

  misc_lambert.p_pj = proj_t (deg2rad * misc_lambert.LoVInDegrees, 
                              deg2rad * misc_lambert.LaDInDegrees, 
                              misc_lambert.projectionCentreFlag == 128 ? -1.0 : +1.0);
  misc_lambert.center_xy = misc_lambert.p_pj.latlonToXy (misc_lambert.p_pj.ref_pt);

  if (! opts.gencoords.on)
    setupCoordinates ();

  if (opts.frame.on)
    setupFrame ();

}

GeometryLambert::~GeometryLambert ()
{
}

int GeometryLambert::latlon2index (float lat, float lon) const
{
  xy_t xy = misc_lambert.p_pj.latlonToXy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy = xy - misc_lambert.center_xy;

  int i = (xy.x / misc_lambert.DxInMetres + misc_lambert.Nux / 2 + 0.5);
  int j = (xy.y / misc_lambert.DyInMetres + misc_lambert.Nuy / 2 + 0.5);

  if ((i < 0) || (i >= grid_lambert.Nx))
    return -1;
  if ((j < 0) || (j >= grid_lambert.Ny))
    return -1;

  return j * grid_lambert.Nx + i;
}

void GeometryLambert::index2latlon (int jglo, float * lat, float * lon) const
{
  int i = jglo % grid_lambert.Nx;
  int j = jglo / grid_lambert.Nx;

  xy_t pt_xy ((i - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (j - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  pt_xy = pt_xy + misc_lambert.center_xy;
  
  latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);
  
  *lon = latlon.lon;
  *lat = latlon.lat;
}

const std::string GeometryLambert::md5 () const
{
  glGrib::MD5 ctx;

  ctx.update (&grid_lambert.Nx, sizeof (grid_lambert.Nx));
  ctx.update (&grid_lambert.Ny, sizeof (grid_lambert.Ny));
  ctx.update (&misc_lambert.Nux, sizeof (misc_lambert.Nux));
  ctx.update (&misc_lambert.Nuy, sizeof (misc_lambert.Nuy));
  ctx.update (&misc_lambert.projectionCentreFlag, sizeof (misc_lambert.projectionCentreFlag));
  ctx.update (&misc_lambert.LaDInDegrees, sizeof (misc_lambert.LaDInDegrees));
  ctx.update (&misc_lambert.LoVInDegrees, sizeof (misc_lambert.LoVInDegrees));
  ctx.update (&misc_lambert.DxInMetres, sizeof (misc_lambert.DxInMetres));
  ctx.update (&misc_lambert.DyInMetres, sizeof (misc_lambert.DyInMetres));

  return ctx.asString ();
}

bool GeometryLambert::isEqual (const Geometry & geom) const
{
  try
    {
      const GeometryLambert & g = dynamic_cast<const GeometryLambert &>(geom);
      return (grid_lambert.Nx                    == g.grid_lambert.Nx)
          && (grid_lambert.Ny                    == g.grid_lambert.Ny)
          && (misc_lambert.Nux                   == g.misc_lambert.Nux)
          && (misc_lambert.Nuy                   == g.misc_lambert.Nuy)
          && (misc_lambert.projectionCentreFlag  == g.misc_lambert.projectionCentreFlag)
          && (misc_lambert.LaDInDegrees          == g.misc_lambert.LaDInDegrees)
          && (misc_lambert.LoVInDegrees          == g.misc_lambert.LoVInDegrees)
          && (misc_lambert.DxInMetres            == g.misc_lambert.DxInMetres)
          && (misc_lambert.DyInMetres            == g.misc_lambert.DyInMetres);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void GeometryLambert::sample (OpenGLBufferPtr<unsigned char> & pp, const unsigned char p0, const int level) const
{
  auto p = pp->map ();
  xy_t pt_sw ((-misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (-misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  xy_t pt_ne ((+misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (+misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  pt_sw = pt_sw + misc_lambert.center_xy;
  pt_ne = pt_ne + misc_lambert.center_xy;
  latlon_t latlon_sw = misc_lambert.p_pj.xy_to_latlon (pt_sw);
  latlon_t latlon_ne = misc_lambert.p_pj.xy_to_latlon (pt_ne);
  
  float Dlat = latlon_ne.lat - latlon_sw.lat;
  float Dlon = latlon_ne.lon - latlon_sw.lon;
  float lat = (latlon_ne.lat + latlon_sw.lat) / 2.0f;
  
  int lat_stride = (grid_lambert.Ny * pi) / (level * Dlat);
  if (lat_stride == 0)
    lat_stride = 1;

  for (int jlat = 0; jlat < grid_lambert.Ny; jlat++)
    {
      int lon_stride = (lat_stride * Dlat) / (Dlon * std::cos (lat));
      if (lon_stride == 0)
        lon_stride = 1;
      for (int jlon = 0; jlon < grid_lambert.Nx; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jlat*grid_lambert.Nx+jlon] = p0;
    }
}

float GeometryLambert::resolution (int level) const 
{
  if (level == 0)
    level = grid_lambert.Ny;
  xy_t pt_sw ((-misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (-misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  xy_t pt_ne ((+misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (+misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  pt_sw = pt_sw + misc_lambert.center_xy;
  pt_ne = pt_ne + misc_lambert.center_xy;
  latlon_t latlon_sw = misc_lambert.p_pj.xy_to_latlon (pt_sw);
  latlon_t latlon_ne = misc_lambert.p_pj.xy_to_latlon (pt_ne);
  float Dlat = latlon_ne.lat - latlon_sw.lat;
  return Dlat / level;
}

void GeometryLambert::applyUVangle (BufferPtr<float> & angle) const 
{

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < grid_lambert.Ny; j++)
    for (int i = 0; i < grid_lambert.Nx; i++)
      {
        xy_j_t pt_xy (xy_t ((i - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
                            (j - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres), 
                      xy_t (1.0, 0.0), xy_t (0.0, 1.0));
        pt_xy.V = pt_xy.V + misc_lambert.center_xy;
        latlon_j_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);

        int p = j * grid_lambert.Nx + i;
        float coslat = std::cos (latlon.V.lat);

        glm::vec2 U (a * coslat * latlon.A.lon, a * latlon.A.lat);
//      glm::vec2 V (a * coslat * latlon.B.lon, a * latlon.B.lat);

        U = glm::normalize (U);
//      V = glm::normalize (V);

        angle[p] += rad2deg * atan2 (U.y, U.x);        
      }
}

void GeometryLambert::getTriangleVertices (int it, int jglo[3]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  int nti = grid.numberOfTriangles / (grid_lambert.Ny - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * grid_lambert.Nx + (i + 0), ind1 = (j + 0) * grid_lambert.Nx + (i + 1); 
  int ind2 = (j + 1) * grid_lambert.Nx + (i + 0), ind3 = (j + 1) * grid_lambert.Nx + (i + 1); 

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind1; jglo[2] = ind3;
    }
}

void GeometryLambert::getTriangleNeighboursXY (int it, int jglo[3], int itri[3], xy_t xy[4]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;                // it is now the rank of the triangle 012
  int nti = grid.numberOfTriangles / (grid_lambert.Ny - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * grid_lambert.Nx + (i + 0), ind1 = (j + 0) * grid_lambert.Nx + (i + 1); 
  int ind2 = (j + 1) * grid_lambert.Nx + (i + 0), ind3 = (j + 1) * grid_lambert.Nx + (i + 1); 

  for (int p = 0, j_ = 0; j_ <= 1; j_++)
  for (int i_ = 0; i_ <= 1; i_++, p++)
    {
      int I = i + i_, J = j + j_;
      xy_t pt_xy ((I - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
                  (J - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
      pt_xy = pt_xy + misc_lambert.center_xy;
      xy[p] = pt_xy;
    }

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
      itri[0] = j > 0 ? it - nti + 1: -1;
      itri[1] = it + 1;
      itri[2] = i > 0 ? it - 1 : -1;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind3; jglo[2] = ind1;
      itri[0] = j < grid_lambert.Ny-2 ? it + nti : -1;
      itri[1] = i < grid_lambert.Nx-2 ? it + 2 : -1;
      itri[2] = it;
    }
}

void GeometryLambert::getTriangleNeighbours 
  (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) 
const
{ 
  bool t021 = (it % 2) == 0;

  xy_t xy[4];

  getTriangleNeighboursXY (it, jglo, itri, xy);

  glm::vec3 pos[4];
  for (int p = 0; p < 4; p++)
    {
      latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (xy[p]);
      pos[p] = glm::vec3 (std::cos (latlon.lon) * std::cos (latlon.lat),
                          std::sin (latlon.lon) * std::cos (latlon.lat),
                                                  std::sin (latlon.lat));
    }

  if (t021)
    {
      xyz[0] = pos[0];
      xyz[1] = pos[1];
      xyz[2] = pos[2];
    }
  else
    {
      xyz[0] = pos[2];
      xyz[1] = pos[3];
      xyz[2] = pos[1];
    }
}

void GeometryLambert::getTriangleNeighbours 
  (int it, int jglo[3], int itri[3], glm::vec2 xy[3]) 
const
{
  bool t021 = (it % 2) == 0;

  xy_t xya[4];

  getTriangleNeighboursXY (it, jglo, itri, xya);

  for (int i = 0; i < 4; i++)
    xya[i] = xya[i] - misc_lambert.center_xy;

  if (t021)
    {
      xy[0] = glm::vec2 (xya[0].x, xya[0].y);     
      xy[1] = glm::vec2 (xya[1].x, xya[1].y);     
      xy[2] = glm::vec2 (xya[2].x, xya[2].y);     
    }
  else
    {
      xy[0] = glm::vec2 (xya[2].x, xya[2].y);
      xy[1] = glm::vec2 (xya[3].x, xya[3].y);
      xy[2] = glm::vec2 (xya[1].x, xya[1].y);
    }

}

bool GeometryLambert::triangleIsEdge (int it) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  it = it / 2;
  int nti = grid.numberOfTriangles / (2 * (grid_lambert.Ny - 1)); // Number of squares in a row
  int i = it % nti;
  int j = it / nti;

  if ((j == 0) && t021)
    return true;
  if ((j == grid_lambert.Ny-2) && (! t021))
    return true;
    
  if ((i == 0) && t021)
    return true;
  if ((i == grid_lambert.Nx-2) && (! t021))
    return true;

  return false;
}

void GeometryLambert::sampleTriangle 
  (BufferPtr<unsigned char> & s, const unsigned char s0, const int level) 
const
{
  xy_t pt_sw ((-misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (-misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  xy_t pt_ne ((+misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
              (+misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
  pt_sw = pt_sw + misc_lambert.center_xy;
  pt_ne = pt_ne + misc_lambert.center_xy;
  latlon_t latlon_sw = misc_lambert.p_pj.xy_to_latlon (pt_sw);
  latlon_t latlon_ne = misc_lambert.p_pj.xy_to_latlon (pt_ne);
  
  float Dlat = latlon_ne.lat - latlon_sw.lat;
  float Dlon = latlon_ne.lon - latlon_sw.lon;
  float lat = (latlon_ne.lat + latlon_sw.lat) / 2.0f;
  
  int lat_stride = abs (level * pi / Dlat);
  lat_stride = std::max (1, lat_stride);
  int lon_stride = 2.0f * (level * 2.0f * Dlat) / (Dlon * std::cos (lat));
  lon_stride = std::max (1, lon_stride);

  int ntpr = 2 * (grid_lambert.Nx - 1);

  for (int jlat = 0; jlat < grid_lambert.Ny-1; jlat++)
    {
      for (int jlon = 0; jlon < grid_lambert.Nx-1; jlon++)
        if ((jlat % lat_stride == 0) && (jlon % lon_stride == 0))
          s[jlat * ntpr + 2 * jlon] = s0;
    }

}

int GeometryLambert::getTriangle (float lon, float lat) const
{
  xy_t xy = misc_lambert.p_pj.latlonToXy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy = xy - misc_lambert.center_xy;

  int i = xy.x / misc_lambert.DxInMetres + misc_lambert.Nux / 2;
  int j = xy.y / misc_lambert.DyInMetres + misc_lambert.Nuy / 2;

  int ntpr = 2 * (grid_lambert.Nx - 1);
  int it = j * ntpr + i * 2;


  {
    int jglo[3], itri[3]; glm::vec3 xyz[3];
    getTriangleNeighbours (it, jglo, itri, xyz);

    for (int i = 0; i < 3; i++)
      printf (" %d %8d %8d\n", i, jglo[i], itri[i]);

    getTriangleNeighbours (it+1, jglo, itri, xyz);

    for (int i = 0; i < 3; i++)
      printf (" %d %8d %8d\n", i, jglo[i], itri[i]);
  }

  return it;
}

const glm::vec2 GeometryLambert::xyz2conformal (const glm::vec3 &) const
{
  throw std::runtime_error  
    (std::string ("GeometryLambert::xyz2conformal not implemented"));
}

const glm::vec3 GeometryLambert::conformal2xyz (const glm::vec2 & xy) const
{
  xy_t pt_xy (xy.x, xy.y);
  pt_xy = pt_xy + misc_lambert.center_xy;

  latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);

  return lonlat2xyz (latlon.lon, latlon.lat);
}


const glm::vec2 GeometryLambert::conformal2latlon (const glm::vec2 & xy) const
{
  xy_t pt_xy (xy.x, xy.y);
  pt_xy = pt_xy + misc_lambert.center_xy;

  latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);

  return glm::vec2 (rad2deg * latlon.lon, rad2deg * latlon.lat);
}

void GeometryLambert::getPointNeighbours 
  (int jglo, std::vector<int> * neigh) 
const
{
  neigh->resize (0);

  if ((jglo < 0) || (grid.numberOfPoints <= jglo))
    return;

  int i = jglo % grid_lambert.Nx;
  int j = jglo / grid_lambert.Nx;

  int iijj[16] = {-1, +1, +0, +1, +1, +1, +1, +0, 
                  +1, -1, +0, -1, -1, -1, -1, +0};

  for (int k = 0; k < 8; k++)
    {
      int i_ = i + iijj[2*k+0];
      int j_ = j + iijj[2*k+1];
      if ((j_ < 0) || (j_ >= grid_lambert.Ny))
        continue;
      if ((i_ < 0) || (i_ >= grid_lambert.Nx))
        continue;
      neigh->push_back (j_ * grid_lambert.Nx + i_);
    }
}


float GeometryLambert::getLocalMeshSize (int jglo) const
{
  return misc_lambert.DxInMetres / a;
}

void GeometryLambert::getView (View * view) const
{
  OptionsView view_opts = view->getOptions (); 
  latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (misc_lambert.center_xy);
  view_opts.lon = latlon.lon * rad2deg;
  view_opts.lat = latlon.lat * rad2deg;

  glm::vec3 xyz[2][2];

  for (int j = 0; j < 2; j++)
    for (int i = 0; i < 2; i++)
      {
        xy_t pt_xy ((i * (grid_lambert.Nx-1) - misc_lambert.Nux / 2) * misc_lambert.DxInMetres, 
                    (j * (grid_lambert.Ny-1) - misc_lambert.Nuy / 2) * misc_lambert.DyInMetres);
        pt_xy = pt_xy + misc_lambert.center_xy;
	latlon_t latlon = misc_lambert.p_pj.xy_to_latlon (pt_xy);
	xyz[i][j] = lonlat2xyz (latlon.lon, latlon.lat);
      }

  float angmax = acos (glm::dot (xyz[0][0], xyz[1][1]));

  view_opts.fov = rad2deg * angmax / view_opts.distance;

  view->setOptions (view_opts);
}

}


