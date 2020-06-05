#include "glGribGeometryLatLon.h"
#include "glGribTrigonometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <openssl/md5.h>

#include <iostream>
#include <stdexcept>

void glGrib::GeometryLatLon::setProgramParameters (glGrib::Program * program) const 
{
#include "shaders/include/geometry/types.h"

  (void)dumm_type;

  if (vertexbuffer != nullptr)
    {
      program->set ("geometry_type", geometry_none);
    }
  else
    {
      program->set ("geometry_type", geometry_latlon);
      program->set ("geometry_latlon_lat0", lat0);
      program->set ("geometry_latlon_lon0", lon0);
      program->set ("geometry_latlon_dlat", dlat);
      program->set ("geometry_latlon_dlon", dlon);
      program->set ("geometry_latlon_Ni",   Ni);
      program->set ("geometry_latlon_Nj",   Nj);
    }
}

int glGrib::GeometryLatLon::size () const
{
  return Ni * Nj;
}

glGrib::GeometryLatLon::GeometryLatLon (glGrib::HandlePtr ghp)
{
  codes_handle * h = ghp == nullptr ? nullptr : ghp->getCodesHandle ();
  codes_get_long (h, "Ni", &Ni);
  codes_get_long (h, "Nj", &Nj);
  codes_get_double (h, "latitudeOfFirstGridPointInDegrees"   , &latitudeOfFirstGridPointInDegrees  );
  codes_get_double (h, "longitudeOfFirstGridPointInDegrees"  , &longitudeOfFirstGridPointInDegrees );
  codes_get_double (h, "latitudeOfLastGridPointInDegrees"    , &latitudeOfLastGridPointInDegrees   );
  codes_get_double (h, "longitudeOfLastGridPointInDegrees"   , &longitudeOfLastGridPointInDegrees  );
  
}

void glGrib::GeometryLatLon::setupCoordinates ()
{
  vertexbuffer = glGrib::OpenGLBufferPtr<float> (2 * numberOfPoints);

  auto lonlat = vertexbuffer->map ();

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < Nj; j++)
    {
      float lat = lat0 - dlat * static_cast<float> (j);
      for (int i = 0; i < Ni; i++)
        {
          float lon = lon0 + dlon * static_cast<float> (i);
          int p = j * Ni + i;
	  lonlat[2*p+0] = lon;
	  lonlat[2*p+1] = lat;
        }
    }

}

void glGrib::GeometryLatLon::setup (glGrib::HandlePtr ghp, const glGrib::OptionsGeometry & o)
{
  opts = o;

  periodic = false;

  dlat = deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees) / (Nj - 1);
  dlon = longitudeOfLastGridPointInDegrees - longitudeOfFirstGridPointInDegrees;
  while (dlon < 0.)
    dlon += 360.;
  while (dlon >= 360.)
    dlon -= 360.;
  dlon /= (Ni - 1);
  dlon *= deg2rad;
  lat0 = deg2rad * latitudeOfFirstGridPointInDegrees;
  lon0 = deg2rad * longitudeOfFirstGridPointInDegrees;

  double ddlon = Ni * dlon - 2 * pi;
    
  periodic = fabs (ddlon) < 1E-2 * dlon;

  // Compute number of triangles
  
  if (periodic)
    numberOfTriangles = 2 * Ni * (Nj - 1);
  else
    numberOfTriangles = 2 * (Ni - 1) * (Nj - 1);
  
  // Generation of triangles
  if (! opts.triangle_strip.on)
    {
      elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (3 * numberOfTriangles);

      auto ind = elementbuffer->map ();

      for (int j = 0, t = 0; j < Nj-1; j++)
        {
          for (int i = 0; i < Ni-1; i++)
            {
              int ind0 = (j + 0) * Ni + (i + 0); int ind1 = (j + 0) * Ni + (i + 1); 
              int ind2 = (j + 1) * Ni + (i + 0); int ind3 = (j + 1) * Ni + (i + 1); 
              ind[3*t+0] = ind0; ind[3*t+1] = ind2; ind[3*t+2] = ind1; t++;
              ind[3*t+0] = ind1; ind[3*t+1] = ind2; ind[3*t+2] = ind3; t++;
            }
          if (periodic)
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
      if (periodic)
        ind_strip_size = (2 * (Ni + 1) + 1) * (Nj - 1);
      else
        ind_strip_size = (2 * Ni + 1) * (Nj - 1);

      elementbuffer = glGrib::OpenGLBufferPtr<unsigned int> (3 * ind_strip_size);

      auto ind_strip = elementbuffer->map ();

      for (int j = 0, t = 0; j < Nj-1; j++)
        {
          for (int i = 0; i < Ni; i++)
            {
              int ind0 = (j + 0) * Ni + (i + 0); 
              int ind2 = (j + 1) * Ni + (i + 0); 
	      ind_strip[t++] = ind0; ind_strip[t++] = ind2;
            }
          if (periodic)
            {
              int ind1 = (j + 0) * Ni + 0; 
              int ind3 = (j + 1) * Ni + 0; 
	      ind_strip[t++] = ind1; ind_strip[t++] = ind3;
            }
	  ind_strip[t++] = 0xffffffff;
        }
    }



  numberOfPoints  = Ni * Nj;

  if (! opts.gencoords.on)
    setupCoordinates ();

  if (opts.frame.on && (! periodic))
    setupFrame ();
}

void glGrib::GeometryLatLon::setupFrame ()
{
  numberOfPoints_frame = 2 * (Ni + Nj - 2);
  vertexbuffer_frame = glGrib::OpenGLBufferPtr<float> (3 * (numberOfPoints_frame + 2));

  auto lonlat = vertexbuffer_frame->map ();

  int p = 0;

  auto push = [&lonlat, &p, this] (int i, int j, int latcst)
  {
    float lat = lat0 - dlat * static_cast<float> (j);
    float lon = lon0 + dlon * static_cast<float> (i);
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

glGrib::GeometryLatLon::~GeometryLatLon ()
{
}

int glGrib::GeometryLatLon::latlon2index (float lat, float lon) const
{
  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float dl = lon - lon0;
  while (dl < 0.)
    dl += 2 * pi;
  while (dl >= 2 * pi)
    dl -= 2 * pi;

  int i = static_cast<int>(0.5 + dl / dlon);
  int j = static_cast<int>(0.5 + (lat0 - lat) / dlat);

  if ((periodic) && (i >= Ni))
    i = 0;

  if ((i < 0) || (i >= Ni))
    return -1;
  if ((j < 0) || (j >= Nj))
    return -1;

  return j * Ni + i;
}

void glGrib::GeometryLatLon::index2latlon (int jglo, float * lat, float * lon) const
{
  int i = jglo % Ni;
  int j = jglo / Ni;

  *lat = lat0 - dlat * static_cast<float> (j);
  *lon = lon0 + dlon * static_cast<float> (i);
}

std::string glGrib::GeometryLatLon::md5 () const
{
  unsigned char out[MD5_DIGEST_LENGTH];
  MD5_CTX c;
  MD5_Init (&c);

  MD5_Update (&c, &Ni, sizeof (Ni));
  MD5_Update (&c, &Nj, sizeof (Nj));
  MD5_Update (&c, &latitudeOfFirstGridPointInDegrees  , sizeof (latitudeOfFirstGridPointInDegrees ));
  MD5_Update (&c, &longitudeOfFirstGridPointInDegrees , sizeof (longitudeOfFirstGridPointInDegrees));
  MD5_Update (&c, &latitudeOfLastGridPointInDegrees   , sizeof (latitudeOfLastGridPointInDegrees  ));
  MD5_Update (&c, &longitudeOfLastGridPointInDegrees  , sizeof (longitudeOfLastGridPointInDegrees ));
  MD5_Final (out, &c);


  return md5string (out);
}

bool glGrib::GeometryLatLon::isEqual (const glGrib::Geometry & geom) const
{
  try
    {
      const glGrib::GeometryLatLon & g = dynamic_cast<const glGrib::GeometryLatLon &>(geom);
      return (Ni                                 == g.Ni)
          && (Nj                                 == g.Nj)
          && (latitudeOfFirstGridPointInDegrees  == g.latitudeOfFirstGridPointInDegrees)
          && (longitudeOfFirstGridPointInDegrees == g.longitudeOfFirstGridPointInDegrees)
          && (latitudeOfLastGridPointInDegrees   == g.latitudeOfLastGridPointInDegrees)
          && (longitudeOfLastGridPointInDegrees  == g.longitudeOfLastGridPointInDegrees);
    }
  catch (const std::bad_cast & e)
    {
      return false;
    }
}

void glGrib::GeometryLatLon::sample (OpenGLBufferPtr<unsigned char> & pp, const unsigned char p0, const int level) const
{
  auto p = pp->map ();

  float Dlat = deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees);
  float Dlon = deg2rad * (longitudeOfLastGridPointInDegrees - longitudeOfFirstGridPointInDegrees);
  float dlat = Dlat / (Nj - 1);
  float lat0 = deg2rad * latitudeOfFirstGridPointInDegrees;

  int lat_stride = (Nj * pi) / (level * Dlat);
  lat_stride = std::max (1, lat_stride);

  for (int jlat = 0; jlat < Nj; jlat++)
    {
      float lat = lat0 - dlat * static_cast<float> (jlat);
      float coslat = cos (lat);
      int lon_stride = abs (2 * (lat_stride * Dlat) / (Dlon * coslat));
      lon_stride = std::max (1, lon_stride);
      for (int jlon = 0; jlon < Ni; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jlat*Ni+jlon] = p0;
    }
}

void glGrib::GeometryLatLon::getTriangleVertices (int it, int jglo[3]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  int nti = numberOfTriangles / (Nj - 1); // Number of triangles in a row
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


void glGrib::GeometryLatLon::getTriangleNeighboursLatLon (int it, int jglo[3], int itri[3], 
		                                          float & xlon0, float & xlat0, 
		                                          float & xlon1, float & xlat1)
const
{
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;                // it is now the rank of the triangle 012
  int nti = numberOfTriangles / (Nj - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Ni + (i + 0), ind1 = periodic && (i == Ni-1) ? (j + 0) * Ni : (j + 0) * Ni + (i + 1); 
  int ind2 = (j + 1) * Ni + (i + 0), ind3 = periodic && (i == Ni-1) ? (j + 1) * Ni : (j + 1) * Ni + (i + 1); 

  xlon0 = lon0 + dlon * static_cast<float> (i + 0);
  xlon1 = lon0 + dlon * static_cast<float> (i + 1);
  xlat0 = lat0 - dlat * static_cast<float> (j + 0);
  xlat1 = lat0 - dlat * static_cast<float> (j + 1);

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

void glGrib::GeometryLatLon::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
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

void glGrib::GeometryLatLon::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec2 merc[3]) const
{
  bool t021 = (it % 2) == 0;
  float xlon0, xlat0, xlat1, xlon1;

  getTriangleNeighboursLatLon (it, jglo, itri, xlon0, xlat0, xlon1, xlat1);

  float mlon0 = xlon0;
  float mlon1 = xlon1;

  if (fabsf (xlat0 - pi / 2.0f) < 1e-06)
    xlat0 = xlat0 - fabsf (dlat) / 2.0f;
  if (fabsf (xlat0 + pi / 2.0f) < 1e-06)
    xlat0 = xlat0 + fabsf (dlat) / 2.0f;
  if (fabsf (xlat1 - pi / 2.0f) < 1e-06)
    xlat1 = xlat1 - fabsf (dlat) / 2.0f;
  if (fabsf (xlat1 + pi / 2.0f) < 1e-06)
    xlat1 = xlat1 + fabsf (dlat) / 2.0f;

  float mlat0 = log (tan (pi / 4.0f + xlat0 / 2.0f));
  float mlat1 = log (tan (pi / 4.0f + xlat1 / 2.0f));

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

bool glGrib::GeometryLatLon::triangleIsEdge (int it) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  it = it / 2;
  int nti = numberOfTriangles / (2 * (Nj - 1)); // Number of squares in a row
  int i = it % nti;
  int j = it / nti;

  if ((j == 0) && t021)
    return true;
  if ((j == Nj-2) && (! t021))
    return true;
    
  if (! periodic)
    {
      if ((i == 0) && t021)
        return true;
      if ((i == Ni-2) && (! t021))
        return true;
    }

  return false;
}

void glGrib::GeometryLatLon::sampleTriangle (unsigned char * s, const unsigned char s0, const int level) const
{
  float Dlat = deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees);
  float Dlon = deg2rad * (longitudeOfLastGridPointInDegrees - longitudeOfFirstGridPointInDegrees);
  float dlat = Dlat / (Nj - 1);
  float lat0 = deg2rad * latitudeOfFirstGridPointInDegrees;

  int lat_stride = abs (level * pi / Dlat);

  int ntpr = periodic ? 2 * Ni : 2 * (Ni - 1);

  for (int jlat = 0; jlat < Nj-1; jlat++)
    {
      float lat = lat0 - dlat * static_cast<float> (jlat);

      if (fabsf (lat - pi / 2.0f) < 1e-6)
        continue;
      if (fabsf (lat + pi / 2.0f) < 1e-6)
        continue;

      float coslat = cos (lat);

      int lon_stride = 2.0f * (level * 2.0f * Dlat) / (Dlon * coslat);

      for (int jlon = 0; jlon < Ni-1; jlon++)
        if ((jlat % lat_stride == 0) && (jlon % lon_stride == 0))
          s[jlat * ntpr + 2 * jlon] = s0;

    }
}

int glGrib::GeometryLatLon::getTriangle (float lon, float lat) const
{
  lat = lat * deg2rad;
  lon = lon * deg2rad;

  float dl = lon - lon0;
  while (dl < 0.)
    dl += 2 * pi;
  while (dl >= 2 * pi)
    dl -= 2 * pi;

  int i = static_cast<int>(dl / dlon);
  int j = static_cast<int>((lat0 - lat) / dlat);

  std::cout << " Ni, Nj = " << Ni << ", " << Nj << std::endl;
  std::cout << " i, j = " << i << ", " << j << std::endl;

  if ((i < 0) || (i >= Ni))
    return -1;
  if ((j < 0) || (j >= Nj))
    return -1;

  int ntpr = periodic ? 2 * Ni : 2 * (Ni - 1);


  int it = j * ntpr + i * 2;
  int it1 = it + 1;


  printf (" it = %d\n", it);

  int itri[3], jglo[3]; glm::vec3 xyz[3];
  getTriangleNeighbours (it, jglo, itri, xyz);
  
  const float rad2deg = 180.0f / pi;
  float xlon = rad2deg * atan2 (xyz[0].y, xyz[0].x);
  float xlat = rad2deg * asin (xyz[0].z);

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

glm::vec2 glGrib::GeometryLatLon::xyz2conformal (const glm::vec3 & xyz) const
{
  float lon = atan2 (xyz.y, xyz.x);
  float lat = asin (xyz.z);
  return glm::vec2 (lon, log (tan (pi / 4.0f + lat / 2.0f)));
}

glm::vec3 glGrib::GeometryLatLon::conformal2xyz (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * atan (exp (merc.y)) - pi / 2.0f;

  return lonlat2xyz (lon, lat);
}

glm::vec2 glGrib::GeometryLatLon::conformal2latlon (const glm::vec2 & merc) const
{
  float lon = merc.x;
  float lat = 2.0f * atan (exp (merc.y)) - pi / 2.0f;
  return glm::vec2 (glm::degrees (lon), glm::degrees (lat));
}

void glGrib::GeometryLatLon::fixPeriodicity (const glm::vec2 & M, glm::vec2 * P, int n) const
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

void glGrib::GeometryLatLon::getPointNeighbours (int jglo, std::vector<int> * neigh) const
{
  neigh->resize (0);

  if ((jglo < 0) || (numberOfPoints <= jglo))
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
      if (! periodic)
        if ((i_ < 0) || (i_ >= Ni))
          continue;
      neigh->push_back (j_ * Ni + i_);
    }
}


float glGrib::GeometryLatLon::getLocalMeshSize (int) const
{
  return dlat;
}


void glGrib::GeometryLatLon::getView (glGrib::View * view) const
{
  if (periodic)
    return;

  glGrib::OptionsView view_opts = view->getOptions (); 

  glm::vec3 xyz[2][2];

  xyz[0][0] = lonlat2xyz (deg2rad * longitudeOfFirstGridPointInDegrees, deg2rad * latitudeOfFirstGridPointInDegrees);
  xyz[0][1] = lonlat2xyz (deg2rad * longitudeOfFirstGridPointInDegrees, deg2rad * latitudeOfLastGridPointInDegrees );
  xyz[1][0] = lonlat2xyz (deg2rad * longitudeOfLastGridPointInDegrees , deg2rad * latitudeOfFirstGridPointInDegrees);
  xyz[1][1] = lonlat2xyz (deg2rad * longitudeOfLastGridPointInDegrees , deg2rad * latitudeOfLastGridPointInDegrees );

  glm::vec3 xyz0 = (xyz[0][0] + xyz[0][1] + xyz[1][0] + xyz[1][1]) / 4.0f;

  float lon0 = atan2 (xyz0.y, xyz0.x);
  float lat0 = asin (xyz0.z);

  view_opts.lon = lon0 * rad2deg;
  view_opts.lat = lat0 * rad2deg;

  float angmax = acos (glm::dot (xyz[0][0], xyz[1][1]));

  view_opts.fov = rad2deg * angmax / view_opts.distance;

  view->setOptions (view_opts);
}

