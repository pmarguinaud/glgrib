#include "glGribGeometryLatLon.h"
#include "glGribTrigonometry.h"
#include "glGribMD5.h"

#include <cmath>

#include <iostream>
#include <stdexcept>

namespace glGrib
{

void GeometryLatLon::setProgramParameters (Program * program) const 
{
#include "shaders/include/geometry/types.h"

  (void)dumm_type;

  if (crds.vertexbuffer != nullptr)
    {
      program->set ("geometry_type", geometry_none);
    }
  else
    {
      program->set ("geometry_type", geometry_latlon);
      program->set ("geometry_latlon_lat0", misc_latlon.lat0);
      program->set ("geometry_latlon_lon0", misc_latlon.lon0);
      program->set ("geometry_latlon_dlat", misc_latlon.dlat);
      program->set ("geometry_latlon_dlon", misc_latlon.dlon);
      program->set ("geometry_latlon_Ni",   grid_latlon.Ni);
      program->set ("geometry_latlon_Nj",   grid_latlon.Nj);
    }
}

int GeometryLatLon::size () const
{
  return grid_latlon.Ni * grid_latlon.Nj;
}

GeometryLatLon::GeometryLatLon (HandlePtr ghp)
{
  codes_handle * h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();
  codes_get_long (h, "Ni", &grid_latlon.Ni);
  codes_get_long (h, "Nj", &grid_latlon.Nj);
  codes_get_double (h, "latitudeOfFirstGridPointInDegrees"   , 
                    &misc_latlon.latitudeOfFirstGridPointInDegrees  );
  codes_get_double (h, "longitudeOfFirstGridPointInDegrees"  , 
                    &misc_latlon.longitudeOfFirstGridPointInDegrees );
  codes_get_double (h, "latitudeOfLastGridPointInDegrees"    , 
                    &misc_latlon.latitudeOfLastGridPointInDegrees   );
  codes_get_double (h, "longitudeOfLastGridPointInDegrees"   , 
                    &misc_latlon.longitudeOfLastGridPointInDegrees  );
  
}

void GeometryLatLon::setupCoordinates ()
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  crds.vertexbuffer = OpenGLBufferPtr<float> (2 * grid.numberOfPoints);

  auto lonlat = crds.vertexbuffer->map ();

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < Nj; j++)
    {
      float lat = misc_latlon.lat0 - misc_latlon.dlat * static_cast<float> (j);
      for (int i = 0; i < Ni; i++)
        {
          float lon = misc_latlon.lon0 + misc_latlon.dlon * static_cast<float> (i);
          int p = j * Ni + i;
	  lonlat[2*p+0] = lon;
	  lonlat[2*p+1] = lat;
        }
    }

}

void GeometryLatLon::setup (HandlePtr ghp, const OptionsGeometry & o)
{
  opts = o;

  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  misc_latlon.periodic = false;

  misc_latlon.dlat = deg2rad * (misc_latlon.latitudeOfFirstGridPointInDegrees 
                              - misc_latlon.latitudeOfLastGridPointInDegrees) / (Nj - 1);
  misc_latlon.dlon = misc_latlon.longitudeOfLastGridPointInDegrees 
                   - misc_latlon.longitudeOfFirstGridPointInDegrees;
  while (misc_latlon.dlon < 0.)
    misc_latlon.dlon += 360.;
  while (misc_latlon.dlon >= 360.)
    misc_latlon.dlon -= 360.;
  misc_latlon.dlon /= (Ni - 1);
  misc_latlon.dlon *= deg2rad;
  misc_latlon.lat0 = deg2rad * misc_latlon.latitudeOfFirstGridPointInDegrees;
  misc_latlon.lon0 = deg2rad * misc_latlon.longitudeOfFirstGridPointInDegrees;

  double ddlon = Ni * misc_latlon.dlon - 2 * pi;
    
  misc_latlon.periodic = std::abs (ddlon) < 1E-2 * misc_latlon.dlon;

  // Compute number of triangles
  
  if (misc_latlon.periodic)
    grid.numberOfTriangles = 2 * Ni * (Nj - 1);
  else
    grid.numberOfTriangles = 2 * (Ni - 1) * (Nj - 1);
  
  // Generation of triangles
  if (! opts.triangle_strip.on)
    {
      grid.elementbuffer = OpenGLBufferPtr<unsigned int> (3 * grid.numberOfTriangles);

      auto ind = grid.elementbuffer->map ();

      for (int j = 0, t = 0; j < Nj-1; j++)
        {
          for (int i = 0; i < Ni-1; i++)
            {
              int ind0 = (j + 0) * Ni + (i + 0); int ind1 = (j + 0) * Ni + (i + 1); 
              int ind2 = (j + 1) * Ni + (i + 0); int ind3 = (j + 1) * Ni + (i + 1); 
              ind[3*t+0] = ind0; ind[3*t+1] = ind2; ind[3*t+2] = ind1; t++;
              ind[3*t+0] = ind1; ind[3*t+1] = ind2; ind[3*t+2] = ind3; t++;
            }
          if (misc_latlon.periodic)
            {
              int ind0 = (j + 0) * Ni + Ni-1; int ind1 = (j + 0) * Ni + 0; 
              int ind2 = (j + 1) * Ni + Ni-1; int ind3 = (j + 1) * Ni + 0; 
              ind[3*t+0] = ind0; ind[3*t+1] = ind2; ind[3*t+2] = ind1; t++;
              ind[3*t+0] = ind1; ind[3*t+1] = ind2; ind[3*t+2] = ind3; t++;
            }
        }
    }
  else
    {
      if (misc_latlon.periodic)
        grid.ind_strip_size = (2 * (Ni + 1) + 1) * (Nj - 1);
      else
        grid.ind_strip_size = (2 * Ni + 1) * (Nj - 1);

      grid.elementbuffer = OpenGLBufferPtr<unsigned int> (3 * grid.ind_strip_size);

      auto ind_strip = grid.elementbuffer->map ();

      for (int j = 0, t = 0; j < Nj-1; j++)
        {
          for (int i = 0; i < Ni; i++)
            {
              int ind0 = (j + 0) * Ni + (i + 0); 
              int ind2 = (j + 1) * Ni + (i + 0); 
	      ind_strip[t++] = ind0; ind_strip[t++] = ind2;
            }
          if (misc_latlon.periodic)
            {
              int ind1 = (j + 0) * Ni + 0; 
              int ind3 = (j + 1) * Ni + 0; 
	      ind_strip[t++] = ind1; ind_strip[t++] = ind3;
            }
	  ind_strip[t++] = OpenGL::restart;
        }
    }



  grid.numberOfPoints  = Ni * Nj;

  if (! opts.gencoords.on)
    setupCoordinates ();

  if (opts.frame.on && (! misc_latlon.periodic))
    setupFrame ();
}

void GeometryLatLon::setupFrame ()
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  frame.numberOfPoints = 2 * (Ni + Nj - 2);
  frame.vertexbuffer = OpenGLBufferPtr<float> (3 * (frame.numberOfPoints + 2));

  auto lonlat = frame.vertexbuffer->map ();

  int p = 0;

  auto push = [&lonlat, &p, this] (int i, int j, int latcst)
  {
    float lat = misc_latlon.lat0 - misc_latlon.dlat * static_cast<float> (j);
    float lon = misc_latlon.lon0 + misc_latlon.dlon * static_cast<float> (i);
    lonlat[3*p+0] = lon;
    lonlat[3*p+1] = lat;
    lonlat[3*p+2] = latcst == 0 ? 1.0f : 0.0f;
    p++;
  };
   
  for (int j = 0; j < Nj-1; j++)
    push (0, j, 1);
  
  for (int i = 0; i < Ni-1; i++)
    push (i, Nj-1, 0);

  for (int j = Nj-1; j >= 1; j--)
    push (Ni-1, j, 1);
  
  for (int i = Ni-1; i >= 1; i--)
    push (i, 0, 0);
  
  for (int j = 0; j < 2; j++)
    push (0, j, 1);
  
}

GeometryLatLon::~GeometryLatLon ()
{
}

int GeometryLatLon::latlon2index (float lat, float lon) const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float dl = lon - misc_latlon.lon0;
  while (dl < 0.)
    dl += 2 * pi;
  while (dl >= 2 * pi)
    dl -= 2 * pi;

  int i = static_cast<int>(0.5 + dl / misc_latlon.dlon);
  int j = static_cast<int>(0.5 + (misc_latlon.lat0 - lat) / misc_latlon.dlat);

  if ((misc_latlon.periodic) && (i >= Ni))
    i = 0;

  if ((i < 0) || (i >= Ni))
    return -1;
  if ((j < 0) || (j >= Nj))
    return -1;

  return j * Ni + i;
}

void GeometryLatLon::index2latlon (int jglo, float * lat, float * lon) const
{
  auto Ni = grid_latlon.Ni;

  int i = jglo % Ni;
  int j = jglo / Ni;

  *lat = misc_latlon.lat0 - misc_latlon.dlat * static_cast<float> (j);
  *lon = misc_latlon.lon0 + misc_latlon.dlon * static_cast<float> (i);
}

const std::string GeometryLatLon::md5 () const
{
  glGrib::MD5 ctx;

  ctx.update (&grid_latlon.Ni, sizeof (grid_latlon.Ni));
  ctx.update (&grid_latlon.Nj, sizeof (grid_latlon.Nj));
  ctx.update (&misc_latlon.latitudeOfFirstGridPointInDegrees  , 
              sizeof (misc_latlon.latitudeOfFirstGridPointInDegrees ));
  ctx.update (&misc_latlon.longitudeOfFirstGridPointInDegrees , 
              sizeof (misc_latlon.longitudeOfFirstGridPointInDegrees));
  ctx.update (&misc_latlon.latitudeOfLastGridPointInDegrees   , 
              sizeof (misc_latlon.latitudeOfLastGridPointInDegrees  ));
  ctx.update (&misc_latlon.longitudeOfLastGridPointInDegrees  , 
              sizeof (misc_latlon.longitudeOfLastGridPointInDegrees ));

  return ctx.asString ();
}

bool GeometryLatLon::isEqual (const Geometry & geom) const
{
  try
    {
      const GeometryLatLon & g = dynamic_cast<const GeometryLatLon &>(geom);
      return (grid_latlon.Ni == g.grid_latlon.Ni)
          && (grid_latlon.Nj == g.grid_latlon.Nj)
          && (misc_latlon.latitudeOfFirstGridPointInDegrees  
         == g.misc_latlon.latitudeOfFirstGridPointInDegrees)
          && (misc_latlon.longitudeOfFirstGridPointInDegrees 
         == g.misc_latlon.longitudeOfFirstGridPointInDegrees)
          && (misc_latlon.latitudeOfLastGridPointInDegrees   
         == g.misc_latlon.latitudeOfLastGridPointInDegrees)
          && (misc_latlon.longitudeOfLastGridPointInDegrees  
         == g.misc_latlon.longitudeOfLastGridPointInDegrees);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void GeometryLatLon::sample (OpenGLBufferPtr<unsigned char> & pp, const unsigned char p0, const int level) const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  auto p = pp->map ();

  float Dlat = deg2rad * (misc_latlon.latitudeOfFirstGridPointInDegrees 
                        - misc_latlon.latitudeOfLastGridPointInDegrees);
  float Dlon = deg2rad * (misc_latlon.longitudeOfLastGridPointInDegrees 
                        - misc_latlon.longitudeOfFirstGridPointInDegrees);
  float dlat = Dlat / (Nj - 1);
  float lat0 = deg2rad * misc_latlon.latitudeOfFirstGridPointInDegrees;

  int lat_stride = (Nj * pi) / (level * Dlat);
  lat_stride = std::max (1, lat_stride);

  for (int jlat = 0; jlat < Nj; jlat++)
    {
      float lat = lat0 - dlat * static_cast<float> (jlat);
      float coslat = std::cos (lat);
      int lon_stride = abs (2 * (lat_stride * Dlat) / (Dlon * coslat));
      lon_stride = std::max (1, lon_stride);
      for (int jlon = 0; jlon < Ni; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jlat*Ni+jlon] = p0;
    }
}

void GeometryLatLon::getTriangleVertices (int it, int jglo[3]) const
{ 
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;
  auto periodic = misc_latlon.periodic;

  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  int nti = grid.numberOfTriangles / (Nj - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Ni + (i + 0), ind1 = periodic && (i == Ni-1) ? (j + 0) * Ni : (j + 0) * Ni + (i + 1); 
  int ind2 = (j + 1) * Ni + (i + 0), ind3 = periodic && (i == Ni-1) ? (j + 1) * Ni : (j + 1) * Ni + (i + 1); 

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind2; jglo[2] = ind1;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind3; jglo[2] = ind1;
    }
}


void GeometryLatLon::getTriangleNeighboursLatLon 
  (int it, int jglo[3], int itri[3], 
   float & xlon0, float & xlat0, 
   float & xlon1, float & xlat1)
const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;
  auto periodic = misc_latlon.periodic;

  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;                // it is now the rank of the triangle 012
  int nti = grid.numberOfTriangles / (Nj - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Ni + (i + 0), ind1 = periodic && (i == Ni-1) ? (j + 0) * Ni : (j + 0) * Ni + (i + 1); 
  int ind2 = (j + 1) * Ni + (i + 0), ind3 = periodic && (i == Ni-1) ? (j + 1) * Ni : (j + 1) * Ni + (i + 1); 

  xlon0 = misc_latlon.lon0 + misc_latlon.dlon * static_cast<float> (i + 0);
  xlon1 = misc_latlon.lon0 + misc_latlon.dlon * static_cast<float> (i + 1);
  xlat0 = misc_latlon.lat0 - misc_latlon.dlat * static_cast<float> (j + 0);
  xlat1 = misc_latlon.lat0 - misc_latlon.dlat * static_cast<float> (j + 1);

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
      itri[0] = j > 0 ? it - nti + 1: -1;
      itri[1] = it + 1;
      itri[2] = i > 0 ? it - 1 : periodic ? it + nti - 1 : -1;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind3; jglo[2] = ind1;
      itri[0] = j < Nj-2 ? it + nti : -1;
      if (periodic)
        itri[1] = i < Ni-1 ? it + 2 : it - nti + 2;
      else
        itri[1] = i < Ni-2 ? it + 2 : -1;
      itri[2] = it;
    }

}

void GeometryLatLon::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
{ 
  bool t021 = (it % 2) == 0;
  float xlon0, xlat0, xlat1, xlon1;

  getTriangleNeighboursLatLon (it, jglo, itri, xlon0, xlat0, xlon1, xlat1);

  if (t021)
    {
      xyz[0] = lonlat2xyz (xlon0, xlat0); 
      xyz[1] = lonlat2xyz (xlon1, xlat0); 
      xyz[2] = lonlat2xyz (xlon0, xlat1); 
    }
  else
    {
      xyz[0] = lonlat2xyz (xlon0, xlat1);
      xyz[1] = lonlat2xyz (xlon1, xlat1);
      xyz[2] = lonlat2xyz (xlon1, xlat0);
    }
}

void GeometryLatLon::getTriangleNeighbours 
  (int it, int jglo[3], int itri[3], glm::vec2 merc[3]) 
const
{
  bool t021 = (it % 2) == 0;
  float xlon0, xlat0, xlat1, xlon1;

  getTriangleNeighboursLatLon (it, jglo, itri, xlon0, xlat0, xlon1, xlat1);

  float mlon0 = xlon0;
  float mlon1 = xlon1;

  if (std::abs (xlat0 - pi / 2.0f) < 1e-06)
    xlat0 = xlat0 - std::abs (misc_latlon.dlat) / 2.0f;
  if (std::abs (xlat0 + pi / 2.0f) < 1e-06)
    xlat0 = xlat0 + std::abs (misc_latlon.dlat) / 2.0f;
  if (std::abs (xlat1 - pi / 2.0f) < 1e-06)
    xlat1 = xlat1 - std::abs (misc_latlon.dlat) / 2.0f;
  if (std::abs (xlat1 + pi / 2.0f) < 1e-06)
    xlat1 = xlat1 + std::abs (misc_latlon.dlat) / 2.0f;

  float mlat0 = std::log (std::tan (pi / 4.0f + xlat0 / 2.0f));
  float mlat1 = std::log (std::tan (pi / 4.0f + xlat1 / 2.0f));

  if (t021)
    {
      merc[0] = glm::vec2 (mlon0, mlat0);
      merc[1] = glm::vec2 (mlon1, mlat0);
      merc[2] = glm::vec2 (mlon0, mlat1);
    }
  else
    {
      merc[0] = glm::vec2 (mlon0, mlat1);
      merc[1] = glm::vec2 (mlon1, mlat1);
      merc[2] = glm::vec2 (mlon1, mlat0);
    }
}

bool GeometryLatLon::triangleIsEdge (int it) const
{ 
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  it = it / 2;
  int nti = grid.numberOfTriangles / (2 * (Nj - 1)); // Number of squares in a row
  int i = it % nti;
  int j = it / nti;

  if ((j == 0) && t021)
    return true;
  if ((j == Nj-2) && (! t021))
    return true;
    
  if (! misc_latlon.periodic)
    {
      if ((i == 0) && t021)
        return true;
      if ((i == Ni-2) && (! t021))
        return true;
    }

  return false;
}

void GeometryLatLon::sampleTriangle 
  (BufferPtr<unsigned char> & s, const unsigned char s0, const int level) 
const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  float Dlat = deg2rad * (misc_latlon.latitudeOfFirstGridPointInDegrees 
                        - misc_latlon.latitudeOfLastGridPointInDegrees);
  float Dlon = deg2rad * (misc_latlon.longitudeOfLastGridPointInDegrees 
                        - misc_latlon.longitudeOfFirstGridPointInDegrees);
  float dlat = Dlat / (Nj - 1);
  float lat0 = deg2rad * misc_latlon.latitudeOfFirstGridPointInDegrees;

  int lat_stride = abs (level * pi / Dlat);

  int ntpr = misc_latlon.periodic ? 2 * Ni : 2 * (Ni - 1);

  for (int jlat = 0; jlat < Nj-1; jlat++)
    {
      float lat = lat0 - dlat * static_cast<float> (jlat);

      if (std::abs (lat - pi / 2.0f) < 1e-6)
        continue;
      if (std::abs (lat + pi / 2.0f) < 1e-6)
        continue;

      float coslat = std::cos (lat);

      int lon_stride = 2.0f * (level * 2.0f * Dlat) / (Dlon * coslat);

      for (int jlon = 0; jlon < Ni-1; jlon++)
        if ((jlat % lat_stride == 0) && (jlon % lon_stride == 0))
          s[jlat * ntpr + 2 * jlon] = s0;

    }
}

int GeometryLatLon::getTriangle (float lon, float lat) const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float dl = lon - misc_latlon.lon0;
  while (dl < 0.)
    dl += 2 * pi;
  while (dl >= 2 * pi)
    dl -= 2 * pi;

  int i = static_cast<int>(dl / misc_latlon.dlon);
  int j = static_cast<int>((misc_latlon.lat0 - lat) / misc_latlon.dlat);

  std::cout << " Ni, Nj = " << Ni << ", " << Nj << std::endl;
  std::cout << " i, j = " << i << ", " << j << std::endl;

  if ((i < 0) || (i >= Ni))
    return -1;
  if ((j < 0) || (j >= Nj))
    return -1;

  int ntpr = misc_latlon.periodic ? 2 * Ni : 2 * (Ni - 1);


  int it = j * ntpr + i * 2;
  int it1 = it + 1;


  printf (" it = %d\n", it);

  int itri[3], jglo[3]; glm::vec3 xyz[3];
  getTriangleNeighbours (it, jglo, itri, xyz);
  
  const float rad2deg = 180.0f / pi;
  float xlon = rad2deg * atan2 (xyz[0].y, xyz[0].x);
  float xlat = rad2deg * std::asin (xyz[0].z);

  printf (" lat = %12.5f lon = %12.5f\n", xlat, xlon);

  printf (" %s %8s %8s\n", "i", "jglo", "itri");
  for (int i = 0; i < 3; i++)
    printf (" %d %8d %8d\n", i, jglo[i], itri[i]);

  printf (" it = %d\n", it1);

  getTriangleNeighbours (it1, jglo, itri, xyz);
  
  printf (" %s %8s %8s\n", "i", "jglo", "itri");
  for (int i = 0; i < 3; i++)
    printf (" %d %8d %8d\n", i, jglo[i], itri[i]);


  return it;
}

const glm::vec2 GeometryLatLon::xyz2conformal (const glm::vec3 & xyz) const
{
  float lon = atan2 (xyz.y, xyz.x);
  float lat = std::asin (xyz.z);
  return glm::vec2 (lon, std::log (std::tan (pi / 4.0f + lat / 2.0f)));
}

const glm::vec3 GeometryLatLon::conformal2xyz (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * std::atan (std::exp (merc.y)) - pi / 2.0f;

  return lonlat2xyz (lon, lat);
}

const glm::vec2 GeometryLatLon::conformal2latlon (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * std::atan (std::exp (merc.y)) - pi / 2.0f;
  return glm::vec2 (glm::degrees (lon), glm::degrees (lat));
}

void GeometryLatLon::fixPeriodicity 
  (const glm::vec2 & M, glm::vec2 * P, int n) 
const
{
  // Fix periodicity issue
  for (int i = 0; i < n; i++)
    {
      while (M.x - P[i].x > pi)
        P[i].x += 2.0f * pi;
      while (P[i].x - M.x > pi)
        P[i].x -= 2.0f * pi;
    }
}

void GeometryLatLon::getPointNeighbours 
  (int jglo, std::vector<int> * neigh) 
const
{
  auto Ni = grid_latlon.Ni;
  auto Nj = grid_latlon.Nj;

  neigh->resize (0);

  if ((jglo < 0) || (grid.numberOfPoints <= jglo))
    return;

  int i = jglo % Ni;
  int j = jglo / Ni;

  int iijj[16] = {-1, +1, +0, +1, +1, +1, +1, +0, 
                  +1, -1, +0, -1, -1, -1, -1, +0};

  for (int k = 0; k < 8; k++)
    {
      int i_ = i + iijj[2*k+0];
      int j_ = j + iijj[2*k+1];
      if ((j_ < 0) || (j_ >= Nj))
        continue;
      if (! misc_latlon.periodic)
        if ((i_ < 0) || (i_ >= Ni))
          continue;
      neigh->push_back (j_ * Ni + i_);
    }
}


float GeometryLatLon::getLocalMeshSize (int) const
{
  return misc_latlon.dlat;
}


void GeometryLatLon::getView (View * view) const
{
  if (misc_latlon.periodic)
    return;

  OptionsView view_opts = view->getOptions (); 

  glm::vec3 xyz[2][2];

  xyz[0][0] = lonlat2xyz (deg2rad * misc_latlon.longitudeOfFirstGridPointInDegrees, 
                          deg2rad * misc_latlon.latitudeOfFirstGridPointInDegrees);
  xyz[0][1] = lonlat2xyz (deg2rad * misc_latlon.longitudeOfFirstGridPointInDegrees, 
                          deg2rad * misc_latlon.latitudeOfLastGridPointInDegrees );
  xyz[1][0] = lonlat2xyz (deg2rad * misc_latlon.longitudeOfLastGridPointInDegrees , 
                          deg2rad * misc_latlon.latitudeOfFirstGridPointInDegrees);
  xyz[1][1] = lonlat2xyz (deg2rad * misc_latlon.longitudeOfLastGridPointInDegrees , 
                          deg2rad * misc_latlon.latitudeOfLastGridPointInDegrees );

  glm::vec3 xyz0 = (xyz[0][0] + xyz[0][1] + xyz[1][0] + xyz[1][1]) / 4.0f;

  float lon0 = atan2 (xyz0.y, xyz0.x);
  float lat0 = std::asin (xyz0.z);

  view_opts.lon = lon0 * rad2deg;
  view_opts.lat = lat0 * rad2deg;

  float angmax = acos (glm::dot (xyz[0][0], xyz[1][1]));

  view_opts.fov = rad2deg * angmax / view_opts.distance;

  view->setOptions (view_opts);
}

}
