#include "glgrib_land.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"
#include "glgrib_earcut.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <omp.h>

#include <glm/gtc/type_ptr.hpp>



static
float getAngle (const glm::vec3 & xyz1, const glm::vec3 & xyz2) 
{
  return acos (std::max (-1.0f, 
               std::min (+1.0f, 
               glm::dot (xyz1, xyz2))));
}

void glgrib_land::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::LAND);
  program->use (); 

  view.setMVP (program);

//glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);

//glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glgrib_land::clear ()
{
  if (isReady ())
    glDeleteVertexArrays (1, &VertexArrayID);
  glgrib_object::clear ();
}

static const float rad2deg = 180.0f / M_PI;
static const float deg2rad = M_PI / 180.0f;
static const float twopi = 2.0f * M_PI;
static const float pi = M_PI;
static const float halfpi = M_PI / 2.0f;

static
glm::vec3 lonlat2xyz (const glm::vec2 & lonlat)
{
  float coslon = cos (lonlat.x), sinlon = sin (lonlat.x);
  float coslat = cos (lonlat.y), sinlat = sin (lonlat.y);
  return glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
}

static
glm::vec2 xyz2lonlat (const glm::vec3 & xyz)
{
  return glm::vec2 (atan2 (xyz.y, xyz.x), asin (xyz.z));
}

class edge_t
{
public:
  edge_t () {}
  int count = 0;
  int rankb = 0;
  float angle;
  void computePointCoordinates (int i, int j, std::vector<glm::vec3> & xyz) const
  {
    const glm::vec3 & u = xyz[i];
    const glm::vec3 w = glm::normalize (glm::cross (xyz[i], xyz[j]));
    const glm::vec3 v = glm::cross (w, u);
    const float ang = getAngle (xyz[i], xyz[j]);

    for (int k = 0; k < count; k++)
      {
        int rank = rankb + k;
        float a = ang * (k + 1) / (count + 1);
        xyz[rank] = cos (a) * u + sin (a) * v;
      }
  }
  void getEdgeSubdivisions (int i, int j, int * r)
  {
    if (i < j)
      {
        r[0] = i;
        for (int l = 0; l < count; l++)
          r[l+1] = rankb + l;
        r[count+1] = j;
      }
    else
      {
        r[0] = i;
        for (int l = 0; l < count; l++)
          r[l+1] = rankb + count - 1 - l;
        r[count+1] = j;
      }


  }

};

class edge_idx_t : public std::map<std::pair<int,int>,edge_t>
{
public:
  bool has (int i, int j) const
  {
    order (i, j);
    return find (std::pair<int,int> (i, j)) != end ();
  }
  edge_t & get (int i, int j) 
  {
    order (i, j);
    if ((i == 8) && (j == 9)) std::cout << "get 8 9" << std::endl;
    edge_idx_t::iterator it = find (std::pair<int,int> (i, j));
    return it->second;
  }
  edge_t & add (int i, int j)
  {
    order (i, j);
    if ((i == 8) && (j == 9)) 
	    std::cout << "add 8 9" << std::endl;
    std::pair<edge_idx_t::iterator,bool> r =
    insert (std::pair<std::pair<int,int>, edge_t> (std::pair<int,int> (i, j), edge_t ()));
    return r.first->second;
  }

  private:
  void order (int & i, int & j) const
  {
    if (j < i) std::swap (i, j);
  }
};

class triangle_t
{
public:
  int rank = -1;
  float sub[3];
  
  int getLongEdges (const float anglemax, const std::vector<unsigned int> & ind, 
                    const std::vector<glm::vec3> & xyz, edge_idx_t * eidx) 
  {
    for (int i = 0; i < 3; i++)
      {
        int j = (i + 1) % 3;
        int rank1 = ind[3*rank+i], rank2 = ind[3*rank+j];

        if (! eidx->has (rank1, rank2))
          {
            float angle = getAngle (xyz[rank1], xyz[rank2]);
            float count = angle / anglemax;
          
            if (count > 1.0f)
              {
                edge_t & e = eidx->add (rank1, rank2);
                e.count = (int)count;
                e.angle = angle;
              }
          }
	else
          {
            edge_t & e = eidx->get (rank1, rank2);
            sub[i] = e.angle / anglemax;
	  }
      }
    
    return getSubTriangles ();
  }
 
  int getSubTriangles () const
  {
    // Number of triangles we can get now
//  int jtri = sub[0] + sub[1] + sub[2] - *std::min_element (sub, sub + 3);
    int jtri = (int)*std::max_element (sub, sub + 3);
    return jtri ? (jtri+1) : jtri;
  }


  void subdivide (edge_idx_t & eidx, std::vector<unsigned int> & ind, int * itrioff)
  {
    int j = (int)(std::max_element (sub, sub + 3) - sub);
    int k = (j + 1) % 3;
    int i = (k + 1) % 3;


    int ri = ind[3*rank+i];
    int rj = ind[3*rank+j];
    int rk = ind[3*rank+k];

    bool dbg = false;
    if (dbg){ std::cout << "subdivide 8 9 : " << rank << std::endl;
	    std::cout << ri << ", " << rj << ", " << rk << std::endl;
    }
    
    edge_t & e = eidx.get (rj, rk);


    int r[e.count+2];

    e.getEdgeSubdivisions (rj, rk, r);

    if (dbg)
    {
      std::cout << ri << ", " << rj << ", " << rk << std::endl;
      printf ("r = ");
      for (int i = 0; i < e.count+2; i++)
        printf (" %8d", r[i]);
      printf ("\n");
    }

    ind[3*rank+i] = ri;
    ind[3*rank+j] = r[0];
    ind[3*rank+k] = r[1];

    if (dbg) {
      std::cout << " ind = " <<
         ind[3*rank+i] << ", " <<
         ind[3*rank+j] << ", " <<
         ind[3*rank+k] << std::endl;
    }

    int n = *itrioff;

    for (int l = 0; l < e.count; l++)
      {
        ind[3*n+0] = ri;
        ind[3*n+1] = r[l+1];
        ind[3*n+2] = r[l+2];

    if (dbg) {
      std::cout << " ind = " <<
        ind[3*n+0] << ", " << 
        ind[3*n+1] << ", " << 
        ind[3*n+2] << ", " << std::endl;
    }
	n++;
      }

    *itrioff = n;

  }

};



static
void subdivideRing (std::vector<float> & lonlat, 
                    std::vector<unsigned int> & ind,
                    int indr1, int indr2, const float angmax, bool openmp)
{

  std::vector<glm::vec3> xyz;
  xyz.resize (lonlat.size () / 2);

#pragma omp parallel for if (openmp)
  for (int i = 0; i < lonlat.size () / 2; i++)
    xyz[i] = lonlat2xyz (glm::vec2 (lonlat[2*i+0], lonlat[2*i+1]));
   
  int itri = 0;

  int indt1 = indr1/3, indt2 = indr2/3;

  std::vector<triangle_t> triangles (indt2-indt1);

#pragma omp parallel for
  for (int i = indt1; i < indt2; i++)
    triangles[i-indt1].rank = i;

  printf ("%d\n", indt2-indt1);

  edge_idx_t eidx;

  // Find edges to be split
  for (int i = indt1; i < indt2; i++)
    itri += triangles[i-indt1].getLongEdges (angmax, ind, xyz, &eidx);

  printf (" itri = %d\n", itri);
  
  int count = 0;

  for (edge_idx_t::iterator it = eidx.begin (); it != eidx.end (); it++)
    {
      edge_t & e = it->second;
      int i = it->first.first;
      int j = it->first.second;
      e.rankb = xyz.size () + count;
      count += e.count;
    }

  // Add extra points
  xyz.resize (xyz.size () + count);

  // Compute coordinates of new points
  for (edge_idx_t::iterator it = eidx.begin (); it != eidx.end (); it++)
    {
      edge_t & e = it->second;
      int i = it->first.first;
      int j = it->first.second;
      e.computePointCoordinates (i, j, xyz);
    }


  // First subdivision

  printf ("count = %d\n", count);


  int ind_off = ind.size ();
  ind.resize (ind_off + 3 * itri);

  int jtri = ind_off/3;


  printf (" jtri = %d\n", jtri);
  int cc = 0;

printf (" indt1 = %d, indt2 = %d\n", indt1, indt2);

  for (int i = indt1; i < indt2; i++)
    {
      if (triangles[i-indt1].getSubTriangles ())
        {
        triangles[i-indt1].subdivide (eidx, ind, &jtri);
	}
    }

  printf (" jtri = %8d\n", jtri);
  printf (" ind.size ()/3 = %8d\n", ind.size ()/3);



  int npts1 = lonlat.size () / 2;
  int npts2 = xyz.size ();


  std::cout << " npts1 = " << npts1 << " npts2 = " << npts2 << std::endl;

  lonlat.resize (2 * npts2);

#pragma omp parallel for if (openmp)
  for (int i = npts1; i < npts2; i++)
    {
      glm::vec2 ll = xyz2lonlat (xyz[i]);
      lonlat[2*i+0] = ll.x;
      lonlat[2*i+1] = ll.y;
    }


}

void glgrib_land::setup (const glgrib_options_land & o)
{
  opts = o;

  int numberOfPoints;
  glgrib_options_lines lopts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  lopts.path = opts.path; lopts.selector = opts.selector;
  glgrib_shapelib::read (lopts, &numberOfPoints, &numberOfLines, &lonlat, 
                         &indl, lopts.selector);



  std::vector<int> offset, length;
  offset.push_back (0);
  for (int i = 0; i < numberOfPoints; i++)
    {
      if (indl[i] == 0xffffffff)
        {
          length.push_back (i - offset.back () - offset.size ());
          offset.push_back (i + 1 - offset.size ());
        }
    }
  length.push_back (numberOfPoints - offset.back () - offset.size ());

  std::vector<int> ord;
  ord.reserve (length.size ());

  for (int i = 0; i < length.size (); i++)
    ord.push_back (i);

  auto comp = [&length] (int i, int j)
  {
    return length[j] < length[i];
  };

  std::sort (ord.begin (), ord.end (), comp);

  // Offset/length for each ring
  std::vector<int> ind_offset (ord.size ());
  std::vector<int> ind_length (ord.size ());

  int ind_size = 0;
  for (int k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
      if (length[j] > 2)
        {
          ind_length[k] = 3 * (length[j] - 2);
          if (k > 0)
            ind_offset[k] = ind_offset[k-1] + ind_length[k-1];
	  else
            ind_offset[0] = 0;
          ind_size += ind_length[k];
        }
    }

  std::vector<unsigned int> ind (ind_size);

  // Process big blocks serially, with OpenMP on inner loops
  int k = 0;
  for (k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
      if (length[j] < 300)
        break;
      if (length[j] > 2)
        glgrib_earcut::processRing (lonlat, offset[j], offset[j]+length[j], 
                                    ind_offset[k], ind_offset[k]+ind_length[k],
                                    &ind, true);
    }

  // Process small blocks in parallel
#pragma omp parallel for
  for (int l = k; l < ord.size (); l++)
    {
      int j = ord[l];
      if (length[j] > 2)
        glgrib_earcut::processRing (lonlat, offset[j], offset[j]+length[j], 
                                    ind_offset[l], ind_offset[l]+ind_length[l],
                                    &ind, false);
    }


  const float angmax = deg2rad * 2.0f;
 if(1)
  for (int k = 0; k < 1; k++)
    {
      int j = ord[k];
      subdivideRing (lonlat, ind, ind_offset[k], ind_offset[k]+ind_length[k], angmax, true);
    }

  numberOfTriangles = ind.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat.size () * sizeof (lonlat[0]), lonlat.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (ind[0]), ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

