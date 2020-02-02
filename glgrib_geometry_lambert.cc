#include "glgrib_geometry_lambert.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <openssl/md5.h>

#include <iostream>
#include <stdexcept>

const double glgrib_geometry_lambert::a = 6371229.0;

int glgrib_geometry_lambert::size () const
{
  return Nx * Ny;
}

glgrib_geometry_lambert::glgrib_geometry_lambert (glgrib_handle_ptr ghp)
{
  codes_handle * h = ghp->getCodesHandle ();
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

void glgrib_geometry_lambert::setup (glgrib_handle_ptr ghp, const glgrib_options_geometry & opts)
{
  codes_handle * h = ghp->getCodesHandle ();

  // Compute number of triangles
  
  numberOfTriangles = 2 * (Nx - 1) * (Ny - 1);
  numberOfPoints  = Nx * Ny;
  
  // Generation of triangles
  
  if (! opts.triangle_strip.on)
    {
      elementbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfTriangles * sizeof (unsigned int));
      unsigned int * ind = (unsigned int *)elementbuffer->map ();
      for (int j = 0, t = 0; j < Ny-1; j++)
        for (int i = 0; i < Nx-1; i++)
          {
            int ind0 = (j + 0) * Nx + (i + 0); int ind1 = (j + 0) * Nx + (i + 1); 
            int ind2 = (j + 1) * Nx + (i + 0); int ind3 = (j + 1) * Nx + (i + 1); 
            ind[3*t+0] = ind1; ind[3*t+1] = ind3; ind[3*t+2] = ind2; t++;
            ind[3*t+0] = ind0; ind[3*t+1] = ind1; ind[3*t+2] = ind2; t++;
          }
    }
  else
    {
      ind_strip_size = (2 * Nx + 5) * (Ny - 1);
      elementbuffer = new_glgrib_opengl_buffer_ptr (ind_strip_size * sizeof (unsigned int));
      unsigned int * ind_strip = (unsigned int *)elementbuffer->map ();

      auto ind0 = [=] (int i, int j) { return (j + 0) * Nx + (i + 0); };
      auto ind2 = [=] (int i, int j) { return (j + 1) * Nx + (i + 0); };

      int t = 0;
      for (int j = 0; j < Ny-1; j++)
        {
          // Control triangle culling : first triangle cannot be part of the strip
          ind_strip[t++] = ind2 (0, j); 
          ind_strip[t++] = ind0 (0, j);
          ind_strip[t++] = ind0 (1, j);
          ind_strip[t++] = 0xffffffff;
          // Following triangles
          for (int i = 0; i < Nx-1; i++)
            {
              ind_strip[t++] = ind2 (i, j); ind_strip[t++] = ind0 (i+1, j);
            }
          // Last triangle
          ind_strip[t++] = ind2 (Nx-1, j);
	  ind_strip[t++] = 0xffffffff;
	}

      if (t >= ind_strip_size)
        abort ();

      for (; t < ind_strip_size; t++)
	ind_strip[t] = 0xffffffff;
      
    }

  elementbuffer->unmap ();


  vertexbuffer = new_glgrib_opengl_buffer_ptr (2 * numberOfPoints * sizeof (float));

  float * lonlat = (float *)vertexbuffer->map ();

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
	lonlat[2*p+0] = latlon.lon;
	lonlat[2*p+1] = latlon.lat;
      }
      

  lonlat = NULL;
  vertexbuffer->unmap ();

  if (opts.frame.on)
    {
      numberOfPoints_frame = 2 * (Nx + Ny - 2);
      vertexbuffer_frame = new_glgrib_opengl_buffer_ptr (3 * (numberOfPoints_frame + 2) 
                                                         * sizeof (float));

      float * lonlat = (float *)vertexbuffer_frame->map ();

      int p = 0;

      auto push = [lonlat, &p, this] (int i, int j, int latcst)
      {
        xy_t pt_xy ((i - Nux / 2) * DxInMetres, (j - Nuy / 2) * DyInMetres);
        pt_xy = pt_xy + center_xy;

        latlon_t latlon = p_pj.xy_to_latlon (pt_xy);

        lonlat[3*p+0] = latlon.lon;
        lonlat[3*p+1] = latlon.lat;
        lonlat[3*p+2] = latcst == 0 ? 1.0f : 0.0f;
        p++;
      };
       
      for (int j = 0; j < Ny-1; j++)
        push (0, j, 1);
      
      for (int i = 0; i < Nx-1; i++)
        push (i, Ny-1, 0);

      for (int j = Ny-1; j >= 1; j--)
        push (Nx-1, j, 1);
      
      for (int i = Nx-1; i >= 1; i--)
        push (i, 0, 0);
      
      for (int j = 0; j < 2; j++)
        push (0, j, 1);
      
      vertexbuffer_frame->unmap ();
    }

}

glgrib_geometry_lambert::~glgrib_geometry_lambert ()
{
}

int glgrib_geometry_lambert::latlon2index (float lat, float lon) const
{
  xy_t xy = p_pj.latlon_to_xy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy = xy - center_xy;

  int i = (xy.x / DxInMetres + Nux / 2 + 0.5);
  int j = (xy.y / DyInMetres + Nuy / 2 + 0.5);

  if ((i < 0) || (i >= Nx))
    return -1;
  if ((j < 0) || (j >= Ny))
    return -1;

  return j * Nx + i;
}

void glgrib_geometry_lambert::index2latlon (int jglo, float * lat, float * lon) const
{
  int i = jglo % Nx;
  int j = jglo / Nx;

  xy_t pt_xy ((i - Nux / 2) * DxInMetres, (j - Nuy / 2) * DyInMetres);
  pt_xy = pt_xy + center_xy;
  
  latlon_t latlon = p_pj.xy_to_latlon (pt_xy);
  
  *lon = latlon.lon;
  *lat = latlon.lat;
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
  
  int lat_stride = (Ny * pi) / (level * Dlat);
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

void glgrib_geometry_lambert::applyUVangle (float * angle) const 
{

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < Ny; j++)
    for (int i = 0; i < Nx; i++)
      {
        xy_j_t pt_xy (xy_t ((i - Nux / 2) * DxInMetres, (j - Nuy / 2) * DyInMetres), 
                      xy_t (1.0, 0.0), xy_t (0.0, 1.0));
        pt_xy.V = pt_xy.V + center_xy;
        latlon_j_t latlon = p_pj.xy_to_latlon (pt_xy);

        int p = j * Nx + i;
        float coslat = cos (latlon.V.lat);

        glm::vec2 U (a * coslat * latlon.A.lon, a * latlon.A.lat);
//      glm::vec2 V (a * coslat * latlon.B.lon, a * latlon.B.lat);

        U = glm::normalize (U);
//      V = glm::normalize (V);

        angle[p] += rad2deg * atan2 (U.y, U.x);        
      }
}

void glgrib_geometry_lambert::getTriangleVertices (int it, int jglo[3]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  int nti = numberOfTriangles / (Ny - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Nx + (i + 0), ind1 = (j + 0) * Nx + (i + 1); 
  int ind2 = (j + 1) * Nx + (i + 0), ind3 = (j + 1) * Nx + (i + 1); 

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind1; jglo[2] = ind3;
    }
}

void glgrib_geometry_lambert::getTriangleNeighboursXY (int it, int jglo[3], int itri[3], xy_t xy[4]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;                // it is now the rank of the triangle 012
  int nti = numberOfTriangles / (Ny - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Nx + (i + 0), ind1 = (j + 0) * Nx + (i + 1); 
  int ind2 = (j + 1) * Nx + (i + 0), ind3 = (j + 1) * Nx + (i + 1); 

  for (int p = 0, j_ = 0; j_ <= 1; j_++)
  for (int i_ = 0; i_ <= 1; i_++, p++)
    {
      int I = i + i_, J = j + j_;
      xy_t pt_xy ((I - Nux / 2) * DxInMetres, (J - Nuy / 2) * DyInMetres);
      pt_xy = pt_xy + center_xy;
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
      itri[0] = j < Ny-2 ? it + nti : -1;
      itri[1] = i < Nx-2 ? it + 2 : -1;
      itri[2] = it;
    }
}

void glgrib_geometry_lambert::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
{ 
  bool t021 = (it % 2) == 0;

  xy_t xy[4];

  getTriangleNeighboursXY (it, jglo, itri, xy);

  glm::vec3 pos[4];
  for (int p = 0; p < 4; p++)
    {
      latlon_t latlon = p_pj.xy_to_latlon (xy[p]);
      pos[p] = glm::vec3 (cos (latlon.lon) * cos (latlon.lat),
                          sin (latlon.lon) * cos (latlon.lat),
                                             sin (latlon.lat));
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

void glgrib_geometry_lambert::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec2 xy[3]) const
{
  bool t021 = (it % 2) == 0;

  xy_t xya[4];

  getTriangleNeighboursXY (it, jglo, itri, xya);

  for (int i = 0; i < 4; i++)
    xya[i] = xya[i] - center_xy;

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

bool glgrib_geometry_lambert::triangleIsEdge (int it) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  it = it / 2;
  int nti = numberOfTriangles / (2 * (Ny - 1)); // Number of squares in a row
  int i = it % nti;
  int j = it / nti;

  if ((j == 0) && t021)
    return true;
  if ((j == Ny-2) && (! t021))
    return true;
    
  if ((i == 0) && t021)
    return true;
  if ((i == Nx-2) && (! t021))
    return true;

  return false;
}

void glgrib_geometry_lambert::sampleTriangle (unsigned char * s, const unsigned char s0, const int level) const
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
  
  int lat_stride = abs (level * pi / Dlat);
  lat_stride = std::max (1, lat_stride);
  int lon_stride = 2.0f * (level * 2.0f * Dlat) / (Dlon * cos (lat));
  lon_stride = std::max (1, lon_stride);

  int ntpr = 2 * (Nx - 1);

  for (int jlat = 0; jlat < Ny-1; jlat++)
    {
      for (int jlon = 0; jlon < Nx-1; jlon++)
        if ((jlat % lat_stride == 0) && (jlon % lon_stride == 0))
          s[jlat * ntpr + 2 * jlon] = s0;
    }

}

int glgrib_geometry_lambert::getTriangle (float lon, float lat) const
{
  xy_t xy = p_pj.latlon_to_xy (latlon_t (lon * deg2rad, lat * deg2rad));
  xy = xy - center_xy;

  int i = xy.x / DxInMetres + Nux / 2;
  int j = xy.y / DyInMetres + Nuy / 2;

  int ntpr = 2 * (Nx - 1);
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

glm::vec2 glgrib_geometry_lambert::xyz2conformal (const glm::vec3 &) const
{
  throw std::runtime_error (std::string ("glgrib_geometry_lambert::xyz2conformal not implemented"));
}

glm::vec3 glgrib_geometry_lambert::conformal2xyz (const glm::vec2 & xy) const
{
  xy_t pt_xy (xy.x, xy.y);
  pt_xy = pt_xy + center_xy;

  latlon_t latlon = p_pj.xy_to_latlon (pt_xy);

  float coslon = cos (latlon.lon), sinlon = sin (latlon.lon);
  float coslat = cos (latlon.lat), sinlat = sin (latlon.lat);

  glm::vec3 xyz;

  xyz.x = coslon * coslat;
  xyz.y = sinlon * coslat;
  xyz.z =          sinlat;

  return xyz;
}


glm::vec2 glgrib_geometry_lambert::conformal2latlon (const glm::vec2 & xy) const
{
  xy_t pt_xy (xy.x, xy.y);
  pt_xy = pt_xy + center_xy;

  latlon_t latlon = p_pj.xy_to_latlon (pt_xy);

  return glm::vec2 (rad2deg * latlon.lon, rad2deg * latlon.lat);
}

void glgrib_geometry_lambert::getPointNeighbours (int jglo, std::vector<int> * neigh) const
{
  neigh->resize (0);

  if ((jglo < 0) || (numberOfPoints <= jglo))
    return;

  int i = jglo % Nx;
  int j = jglo / Nx;

  int iijj[16] = {-1, +1, +0, +1, +1, +1, +1, +0, 
                  +1, -1, +0, -1, -1, -1, -1, +0};

  for (int k = 0; k < 8; k++)
    {
      int i_ = i + iijj[2*k+0];
      int j_ = j + iijj[2*k+1];
      if ((j_ < 0) || (j_ >= Ny))
        continue;
      if ((i_ < 0) || (i_ >= Nx))
        continue;
      neigh->push_back (j_ * Nx + i_);
    }
}


float glgrib_geometry_lambert::getLocalMeshSize (int jglo) const
{
  return DxInMetres / a;
}

void glgrib_geometry_lambert::getView (glgrib_view * view) const
{
  glgrib_options_view view_opts = view->getOptions (); 
  latlon_t latlon = p_pj.xy_to_latlon (center_xy);
  view_opts.lon = latlon.lon * rad2deg;
  view_opts.lat = latlon.lat * rad2deg;

  glm::vec3 xyz[2][2];

  for (int j = 0; j < 2; j++)
    for (int i = 0; i < 2; i++)
      {
        xy_t pt_xy ((i * (Nx-1) - Nux / 2) * DxInMetres, (j * (Ny-1) - Nuy / 2) * DyInMetres);
        pt_xy = pt_xy + center_xy;
	latlon_t latlon = p_pj.xy_to_latlon (pt_xy);
        float coslon = cos (latlon.lon), sinlon = sin (latlon.lon);
        float coslat = cos (latlon.lat), sinlat = sin (latlon.lat);
        xyz[i][j].x = coslon * coslat;
        xyz[i][j].y = sinlon * coslat;
        xyz[i][j].z =          sinlat;
      }

  float angmax = acos (glm::dot (xyz[0][0], xyz[1][1]));

  view_opts.fov = rad2deg * angmax / view_opts.distance;

  view->setOptions (view_opts);
}




