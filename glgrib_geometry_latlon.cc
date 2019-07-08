#include "glgrib_geometry_latlon.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glgrib_load.h"
#include <openssl/md5.h>

#include <iostream>
#include <stdexcept>

const double glgrib_geometry_latlon::rad2deg = 180.0 / M_PI;
const double glgrib_geometry_latlon::deg2rad = M_PI / 180.0;

void glgrib_geometry_latlon::gencoords (float * px, float * py) const
{
  throw std::runtime_error (std::string ("glgrib_geometry_latlon::gencoords not implemented"));
}

void glgrib_geometry_latlon::genlatlon (float * plat, float * plon) const
{
  throw std::runtime_error (std::string ("glgrib_geometry_latlon::genlatlon not implemented"));
}

int glgrib_geometry_latlon::size () const
{
  return Ni * Nj;
}

glgrib_geometry_latlon::glgrib_geometry_latlon (codes_handle * h)
{
  codes_get_long (h, "Ni", &Ni);
  codes_get_long (h, "Nj", &Nj);
  codes_get_double (h, "latitudeOfFirstGridPointInDegrees"   , &latitudeOfFirstGridPointInDegrees  );
  codes_get_double (h, "longitudeOfFirstGridPointInDegrees"  , &longitudeOfFirstGridPointInDegrees );
  codes_get_double (h, "latitudeOfLastGridPointInDegrees"    , &latitudeOfLastGridPointInDegrees   );
  codes_get_double (h, "longitudeOfLastGridPointInDegrees"   , &longitudeOfLastGridPointInDegrees  );
  
}

void glgrib_geometry_latlon::init (codes_handle * h, const float orography)
{
  float * xyz = NULL;
  unsigned int * ind = NULL;
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

  double ddlon = lon0 + (Ni + 1) * dlon - 2 * M_PI;
  while (ddlon < 0.)
    ddlon += 2 * M_PI;
  while (ddlon >= 2. * M_PI)
    ddlon -= 2 * M_PI;
    
  periodic = fabs (ddlon) < 1E-2;

  // Compute number of triangles
  
  if (periodic)
    numberOfTriangles = 2 * Ni * (Nj - 1);
  else
    numberOfTriangles = 2 * (Ni - 1) * (Nj - 1);
  
  ind = (unsigned int *)malloc (3 * numberOfTriangles * sizeof (unsigned int));
  // Generation of triangles
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

  xyz = (float *)malloc (3 * sizeof (float) * Ni * Nj);
  numberOfPoints  = Ni * Nj;

  double sinlon[Ni];
  double coslon[Ni];

#pragma omp parallel for
  for (int i = 0; i < Ni; i++)
    {
      float lon = lon0 + dlon * (float)i;
      coslon[i] = cos (lon); 
      sinlon[i] = sin (lon);
    }

  // Generation of coordinates
#pragma omp parallel for
  for (int j = 0; j < Nj; j++)
    {
      float lat = lat0 - dlat * (float)j;
      float coslat = cos (lat), sinlat = sin (lat);
      for (int i = 0; i < Ni; i++)
        {
          int p = j * Ni + i;
          xyz[3*p+0] = coslon[i] * coslat;
          xyz[3*p+1] = sinlon[i] * coslat;
          xyz[3*p+2] =             sinlat;
        }
    }
      
  vertexbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfPoints * sizeof (float), xyz);
  elementbuffer = new_glgrib_opengl_buffer_ptr (3 * numberOfTriangles * sizeof (unsigned int), ind);

  free (xyz); xyz = NULL;
  free (ind); ind = NULL;
}

glgrib_geometry_latlon::~glgrib_geometry_latlon ()
{
}

int glgrib_geometry_latlon::latlon2index (float lat, float lon) const
{
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

  return j * Ni + i;
}

std::string glgrib_geometry_latlon::md5 () const
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

bool glgrib_geometry_latlon::isEqual (const glgrib_geometry & geom) const
{
  try
    {
      const glgrib_geometry_latlon & g = dynamic_cast<const glgrib_geometry_latlon &>(geom);
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

void glgrib_geometry_latlon::sample (unsigned char * p, const unsigned char p0, const int level) const
{
  float Dlat = deg2rad * (latitudeOfFirstGridPointInDegrees - latitudeOfLastGridPointInDegrees);
  float Dlon = deg2rad * (longitudeOfLastGridPointInDegrees - longitudeOfFirstGridPointInDegrees);
  float dlat = Dlat / (Nj - 1);
  float lat0 = deg2rad * latitudeOfFirstGridPointInDegrees;

  int lat_stride = (Nj * M_PI) / (level * Dlat);

  for (int jlat = 0; jlat < Nj; jlat++)
    {
      float lat = lat0 + dlat * (float)jlat;
      float coslat = cos (lat);
      int lon_stride = 2 * (lat_stride * Dlat) / (Dlon * coslat);
      for (int jlon = 0; jlon < Ni; jlon++)
        if ((jlat % lat_stride != 0) || (jlon % lon_stride != 0))
          p[jlat*Ni+jlon] = p0;
    }
}

void glgrib_geometry_latlon::getTriangleVertices (int it, int jglo[3]) const
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
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind3; jglo[2] = ind1;
    }

  if (ind0 >= numberOfPoints) abort ();
  if (ind1 >= numberOfPoints) abort ();
  if (ind2 >= numberOfPoints) abort ();
  if (ind3 >= numberOfPoints) abort ();
}

void glgrib_geometry_latlon::getTriangleNeighbours (int it, int jglo[3], int itri[3], glm::vec3 xyz[3]) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;                // it is now the rank of the triangle 012
  int nti = numberOfTriangles / (Nj - 1); // Number of triangles in a row
  int i = (it % nti) / 2;
  int j = (it / nti);
  int ind0 = (j + 0) * Ni + (i + 0), ind1 = periodic && (i == Ni-1) ? (j + 0) * Ni : (j + 0) * Ni + (i + 1); 
  int ind2 = (j + 1) * Ni + (i + 0), ind3 = periodic && (i == Ni-1) ? (j + 1) * Ni : (j + 1) * Ni + (i + 1); 

  float coslon0 = cos (lon0 + dlon * (float)(i + 0)), sinlon0 = sin (lon0 + dlon * (float)(i + 0));
  float coslon1 = cos (lon0 + dlon * (float)(i + 1)), sinlon1 = sin (lon0 + dlon * (float)(i + 1));
  float coslat0 = cos (lat0 - dlat * (float)(j + 0)), sinlat0 = sin (lat0 - dlat * (float)(j + 0));
  float coslat1 = cos (lat0 - dlat * (float)(j + 1)), sinlat1 = sin (lat0 - dlat * (float)(j + 1));

  if (t021)
    {
      jglo[0] = ind0; jglo[1] = ind1; jglo[2] = ind2;
      itri[0] = j > 0 ? it - nti + 1: -1;
      itri[1] = it + 1;
      itri[2] = i > 0 ? it - 1 : periodic ? it + nti - 1 : -1;
      xyz[0] = glm::vec3 (coslon0 * coslat0, sinlon0 * coslat0, sinlat0);
      xyz[1] = glm::vec3 (coslon1 * coslat0, sinlon1 * coslat0, sinlat0);
      xyz[2] = glm::vec3 (coslon0 * coslat1, sinlon0 * coslat1, sinlat1);
    }
  else
    {
      jglo[0] = ind2; jglo[1] = ind3; jglo[2] = ind1;
      itri[0] = j < Nj-2 ? it + nti : -1;
      itri[1] = i < Ni-2 ? it + 2 : periodic ? it - nti + 1 : -1;
      itri[2] = it;
      xyz[0] = glm::vec3 (coslon0 * coslat1, sinlon0 * coslat1, sinlat1);
      xyz[1] = glm::vec3 (coslon1 * coslat1, sinlon1 * coslat1, sinlat1);
      xyz[2] = glm::vec3 (coslon1 * coslat0, sinlon1 * coslat0, sinlat0);
    }

  if (ind0 >= numberOfPoints) abort ();
  if (ind1 >= numberOfPoints) abort ();
  if (ind2 >= numberOfPoints) abort ();
  if (ind3 >= numberOfPoints) abort ();

  if ((itri[0] >= 0) && (itri[0] >= numberOfTriangles)) abort ();
  if ((itri[1] >= 0) && (itri[1] >= numberOfTriangles)) abort ();
  if ((itri[2] >= 0) && (itri[2] >= numberOfTriangles)) abort ();
  

}

bool glgrib_geometry_latlon::triangleIsEdge (int it) const
{ 
  bool t021 = (it % 2) == 0;
  it = t021 ? it : it - 1;
  int nti = numberOfTriangles / (2 * (Nj - 1)); // Number of triangles in a row
  int i = it % nti;
  int j = it / nti;

  if ((j == 0) || (j == Nj-1))
    return true;
    
  if (((i == 0) || (j == Ni-1)) && (! periodic))
    return true;

  return false;
}
