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
    edge_idx_t::iterator it = find (std::pair<int,int> (i, j));
    return it->second;
  }
  edge_t & add (int i, int j)
  {
    order (i, j);
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
  float count[3];
  
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
            count[i] = angle / anglemax;
          
            if (count[i] > 1.0f)
              {
                edge_t & e = eidx->add (rank1, rank2);
                e.count = (int)count[i];
                e.angle = angle;
              }
          }
	else
          {
            edge_t & e = eidx->get (rank1, rank2);
            count[i] = e.angle / anglemax;
	  }
      }
    
    return getSubTriangles ();
  }

  int getSubTriangles () const
  {
    int ord[3] = {0, 1, 2};
    std::sort (ord, ord + 3, [this] (int i, int j) { return this->count[j] < this->count[i]; });
    // Number of triangles we can get now
    if (count[ord[2]] > 1)
      return 1 + (int)count[ord[0]] + (int)count[ord[1]];
    else if (count[ord[1]] > 1)
      return 1 + (int)count[ord[0]] + (int)count[ord[1]];
    else if (count[ord[0]] > 1)
      return 1 + (int)count[ord[0]];
    return 0;
  }

  void subdivide2 (edge_idx_t & eidx, std::vector<unsigned int> & ind, int * itrioff, int ord[3])
  {
    int k = ord[2];
    int i = (k + 1) % 3;
    int j = (i + 1) % 3;

    int ri = ind[3*rank+i];
    int rj = ind[3*rank+j];
    int rk = ind[3*rank+k];

    edge_t & ekj = eidx.get (rk, rj);
    edge_t & eij = eidx.get (ri, rj);

    int nkj = ekj.count+2, lkj = 0; int rkj[nkj]; ekj.getEdgeSubdivisions (rk, rj, rkj);
    int nij = eij.count+2, lij = 0; int rij[nij]; eij.getEdgeSubdivisions (ri, rj, rij);

    auto cmp = [nij, nkj] (int lkj, int lij) { return lkj * nij < lij * nkj; };

    int n = *itrioff;

    while ((lkj < nkj-2) || (lij < nij-2))
      {
        if (cmp (lkj+1, lij+1))
          {
            ind[3*n+0] = rkj[lkj+1];
            ind[3*n+1] = rkj[lkj];
            ind[3*n+2] = rij[lij];
	    lkj++;
	  }
	else
          {
            ind[3*n+0] = rij[lij+1];
            ind[3*n+1] = rkj[lkj];
            ind[3*n+2] = rij[lij];
	    lij++;
          }
	n++;
      }

    ind[3*rank+j] = 0;
    ind[3*rank+k] = 0;
    ind[3*rank+i] = 0;

    ind[3*n+0] = rj;
    ind[3*n+1] = rkj[nkj-2];
    ind[3*n+2] = rij[nij-2];

    n++;

    *itrioff = n;
  }
  void subdivide1 (edge_idx_t & eidx, std::vector<unsigned int> & ind, int * itrioff, int ord[3])
  {
    int j = ord[0];
    int k = (j + 1) % 3;
    int i = (k + 1) % 3;

    int ri = ind[3*rank+i];
    int rj = ind[3*rank+j];
    int rk = ind[3*rank+k];

    edge_t & e = eidx.get (rj, rk);

    int r[e.count+2];

    e.getEdgeSubdivisions (rj, rk, r);

    ind[3*rank+i] = 0;
    ind[3*rank+j] = 0;
    ind[3*rank+k] = 0;

    int n = *itrioff;

    ind[3*n+0] = ri;
    ind[3*n+1] = r[0];
    ind[3*n+2] = r[1];

    n++;

    for (int l = 0; l < e.count; l++)
      {
        ind[3*n+0] = ri;
        ind[3*n+1] = r[l+1];
        ind[3*n+2] = r[l+2];
	n++;
      }

    *itrioff = n;
  }

  void subdivide (edge_idx_t & eidx, std::vector<unsigned int> & ind, int * itrioff)
  {
    int ord[3] = {0, 1, 2};
    std::sort (ord, ord + 3, [this] (int i, int j) { return this->count[j] < this->count[i]; });

    if (count[ord[2]] > 1)
      subdivide2 (eidx, ind, itrioff, ord);
    else if (count[ord[1]] > 1)
      subdivide2 (eidx, ind, itrioff, ord);
    else if (count[ord[0]] > 1)
      subdivide1 (eidx, ind, itrioff, ord);
  }

};



static
void subdivideRing1 (std::vector<glm::vec3> & xyz,
                     std::vector<unsigned int> & ind,
                     int indr1, int indr2, 
		     const float angmax)
{

  int indt1 = indr1/3, indt2 = indr2/3;

  std::vector<triangle_t> triangles (indt2-indt1);

  for (int i = indt1; i < indt2; i++)
    triangles[i-indt1].rank = i;

  edge_idx_t eidx;

  int itri = 0;

  // Find edges to be split
  for (int i = indt1; i < indt2; i++)
    itri += triangles[i-indt1].getLongEdges (angmax, ind, xyz, &eidx);

  int ixyz = 0;

  for (edge_idx_t::iterator it = eidx.begin (); it != eidx.end (); it++)
    {
      edge_t & e = it->second;
      int i = it->first.first, j = it->first.second;
      e.rankb = xyz.size () + ixyz;
      ixyz += e.count;
    }

  // Add extra points
  xyz.resize (xyz.size () + ixyz);

  // Compute coordinates of new points
  for (edge_idx_t::iterator it = eidx.begin (); it != eidx.end (); it++)
    {
      edge_t & e = it->second;
      int i = it->first.first, j = it->first.second;
      e.computePointCoordinates (i, j, xyz);
    }

  int ind_off = ind.size ();
  ind.resize (ind_off + 3 * itri);

  int jtri = ind_off / 3;

  for (int i = indt1; i < indt2; i++)
    if (triangles[i-indt1].getSubTriangles ())
      triangles[i-indt1].subdivide (eidx, ind, &jtri);

}

static
void subdivideRing (std::vector<float> & lonlat, 
                    std::vector<unsigned int> & ind,
		    int indp1, int indp2,
                    int indr1, int indr2, 
		    const float angmax)
{
  std::vector<glm::vec3> xyz;
  xyz.resize (lonlat.size () / 2);

#pragma omp parallel for 
  for (int i = 0; i < lonlat.size () / 2; i++)
    xyz[i] = lonlat2xyz (glm::vec2 (lonlat[2*i+0], lonlat[2*i+1]));

  printf (" indp1 = %d, indp2 = %d\n", indp1, indp2);

  std::cout << " min (ind) = " << *std::min_element (ind.begin () + indr1, ind.begin () + indr2) << std::endl;
  std::cout << " max (ind) = " << *std::max_element (ind.begin () + indr1, ind.begin () + indr2) << std::endl;
   
  int ind_off = ind.size ();

  subdivideRing1 (xyz, ind, indr1, indr2, angmax);

  subdivideRing1 (xyz, ind, ind_off, ind.size (), angmax);

  int lonlat_size = lonlat.size () / 2;
  lonlat.resize (2 * lonlat_size + 2 * (xyz.size () - lonlat_size));
  
  for (int i = lonlat_size; i < xyz.size (); i++)
    {
      glm::vec2 ll = xyz2lonlat (xyz[i]);
      lonlat[2*i+0] = ll.x; lonlat[2*i+1] = ll.y;
    }

}


class subdivideRing_t
{
public:

  subdivideRing_t () {}

  void init (const std::vector<float> & lonlat, 
             const std::vector<unsigned int> & ind,
             int _indp1, int _indp2,
             int _indr1, int _indr2)
  {
    indp1 = _indp1; indp2 = _indp2; 
    indr1 = _indr1; indr2 = _indr2;

    xyz1.resize (indp2-indp1);
    ind1.resize (indr2-indr1);
    indt1 = indr1/3; 
    indt2 = indr2/3;
    for (int i = indp1; i < indp2; i++)
      xyz1[i-indp1] = lonlat2xyz (glm::vec2 (lonlat[2*i+0], lonlat[2*i+1]));

    for (int i = indr1; i < indr2; i++)
      ind1[i-indr1] = ind[i]-indp1;

    ind1_size = ind1.size (); 
    xyz1_size = xyz1.size (); 

  }
 
  void subdivide (float angmax)
  {
    subdivideRing1 (xyz1, ind1,         0, ind1.size (), angmax);
    subdivideRing1 (xyz1, ind1, ind1_size, ind1.size (), angmax);
  }

  void append (std::vector<float> & lonlat, std::vector<unsigned int> & ind,
               int points_offset, int triangles_offset)
  {
    // Append new points
    for (int i = xyz1_size; i < xyz1.size (); i++)
      {   
        glm::vec2 ll = xyz2lonlat (xyz1[i]);
        int j = points_offset + i - xyz1_size;
        lonlat[2*j+0] = ll.x; lonlat[2*j+1] = ll.y;
      }   
   
    // Append new triangles
    for (int i = ind1_size; i < ind1.size (); i++)
      {   
        int j = triangles_offset + i - ind1_size;
        // Two cases 
        if (ind1[i] < xyz1_size)     // This vertex already existed
          ind[j] = ind1[i] + indp1;
        else                         // New vertex
          ind[j] = ind1[i] - xyz1_size + points_offset;
      }   
   
    // Discard some of the old triangles
    for (int i = indt1; i < indt2; i++)
      {   
        int j = i - indt1;
        if ((ind1[3*j+0] == ind1[3*j+1]) && (ind1[3*j+1] == ind1[3*j+2]))
          ind[3*i+0] = ind[3*i+1] = ind[3*i+2] = 0;
      }   

  }

  int getPointsLength () const
  {
    return xyz1.size () - xyz1_size;
  }

  int getTrianglesLength () const
  {
    return ind1.size () - ind1_size;
  }

  int indt1, indt2;
  int indp1, indp2;
  int indr1, indr2;
  std::vector<glm::vec3> xyz1;
  std::vector<unsigned int> ind1;

  int ind1_size;
  int xyz1_size;
  
};



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
  length.push_back (numberOfPoints - offset.back () - 1);

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
//#pragma omp parallel for
  for (int l = k; l < ord.size (); l++)
    {
      int j = ord[l];
      if (length[j] > 2)
        glgrib_earcut::processRing (lonlat, offset[j], offset[j]+length[j], 
                                    ind_offset[l], ind_offset[l]+ind_length[l],
                                    &ind, false);
    }


  const float angmax = deg2rad * 1.0f;

 if(0)
   {
     std::vector<subdivideRing_t> sr (ord.size ());

     for (int k = 0; k < ord.size (); k++)
       {
         int j = ord[k];
         sr[k].init (lonlat, ind, offset[j], offset[j]+length[j], 
                     ind_offset[k], ind_offset[k]+ind_length[k]);
         sr[k].subdivide (angmax);
       }

     std::vector<int> points_offset    (ord.size ());
     std::vector<int> triangles_offset (ord.size ());

     points_offset   [0] = lonlat.size () / 2;
     triangles_offset[0] = ind.size ();

     for (int k = 1; k < ord.size (); k++)
       {
         points_offset   [k] = points_offset   [k-1] + sr[k-1].getPointsLength    ();
	 triangles_offset[k] = triangles_offset[k-1] + sr[k-1].getTrianglesLength ();
       }

     lonlat.resize (lonlat.size () + 2 * (points_offset.back () + sr.back ().getPointsLength ()));
     ind.resize (ind.size () + triangles_offset.back () + sr.back ().getTrianglesLength ());

     for (int k = 0; k < ord.size (); k++)
       sr[k].append (lonlat, ind, points_offset[k], triangles_offset[k]);
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

