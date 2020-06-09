#include "glGribSubdivide.h"
#include "glGribTrigonometry.h"

#include <math.h>
#include <iostream>
#include <map>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>


namespace
{

float getAngle (const glm::vec3 & xyz1, const glm::vec3 & xyz2) 
{
  return acos (std::max (-1.0f, 
               std::min (+1.0f, 
               glm::dot (xyz1, xyz2))));
}

// Triangle edge
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


// Edge index : quickly find the edge object for a vertices pair
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

// Triangle object : manage edges, subdivide
class triangle_t
{
public:
  int rank = -1;
  float count[3];
  
  // Process edges which are too long : create an edge object and add it to the index
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
                e.count = static_cast<int>(count[i]);
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
      return 1 + static_cast<int> (count[ord[0]]) + static_cast<int> (count[ord[1]]);
    else if (count[ord[1]] > 1)
      return 1 + static_cast<int> (count[ord[0]]) + static_cast<int> (count[ord[1]]);
    else if (count[ord[0]] > 1)
      return 1 + static_cast<int> (count[ord[0]]);
    return 0;
  }

  // Subdivide triangle with two long edges
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

  // Subdivide triangle with a single long edge
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



// Subdivide a single ring

void subdivideRing1 (std::vector<glm::vec3> & xyz,
                     std::vector<unsigned int> & ind,
                     int indr1, int indr2, 
		     const float angmax)
{

  int indt1 = indr1/3, indt2 = indr2/3;

  std::vector<triangle_t> triangles (indt2-indt1);

  for (int i = indt1; i < indt2; i++)
    triangles[i-indt1].rank = i;

  // Create edge index
  edge_idx_t eidx;

  int itri = 0;

  // Find edges to be split
  for (int i = indt1; i < indt2; i++)
    itri += triangles[i-indt1].getLongEdges (angmax, ind, xyz, &eidx);

  int ixyz = 0;

  for (edge_idx_t::iterator it = eidx.begin (); it != eidx.end (); it++)
    {
      edge_t & e = it->second;
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

  // Subdivide triangle
  for (int i = indt1; i < indt2; i++)
    if (triangles[i-indt1].getSubTriangles ())
      triangles[i-indt1].subdivide (eidx, ind, &jtri);

}

}

void glGrib::Subdivide::init (const std::vector<float> & lonlat, 
                             const std::vector<unsigned int> & ind,
                             int indpb, int indpl,
                             int indrb, int indrl)
{
  // Init subdivider : copy coordinates and triangle indices

  indp1 = indpb; indp2 = indpb + indpl; 
  indr1 = indrb; indr2 = indrb + indrl;

  indt1 = indr1/3; 
  indt2 = indr2/3;


  if (1)
    {
      // Check for triangles validity : all points must lie 
      // in the interval indp1 .. indp2, except for degenerate 
      // triangles (those where all three points are equal)
      for (int indt = indt1; indt < indt2; indt++)
        {
          unsigned int in[3] = {ind[3*indt+0], ind[3*indt+1], ind[3*indt+2]};
          if ((in[0] == in[1]) && (in[1] == in[2]))
            continue;
          for (int i = 0; i < 3; i++)
            if ((static_cast<int> (in[i]) < indp1) || (static_cast<int> (in[i]) > indp2))
              abort ();
        }
    }
  else
    {
      int pmin = *std::min_element (ind.begin () + indr1, ind.begin () + indr2);
      int pmax = *std::max_element (ind.begin () + indr1, ind.begin () + indr2);
      if ((pmin < indp1) || (pmin >= indp2))
        abort ();
      if ((pmax < indp1) || (pmax >= indp2))
        abort ();
    }

  xyz1.resize (indp2-indp1);
  ind1.resize (indr2-indr1);

  for (int i = indp1; i < indp2; i++)
    xyz1[i-indp1] = lonlat2xyz (glm::vec2 (lonlat[2*i+0], lonlat[2*i+1]));

  // Change indices base index
  for (int i = indr1; i < indr2; i++)
    {
      if (static_cast<int>(ind[i]) < indp1)
        ind1[i-indr1] = 0;
      else
        ind1[i-indr1] = ind[i]-indp1;
    }

  ind1_size = ind1.size (); 
  xyz1_size = xyz1.size (); 

}

void glGrib::Subdivide::subdivide (float angmax)
{
  // Two steps

  // 1- Perform 1 and 2-edge subdivisions
  subdivideRing1 (xyz1, ind1,         0, ind1.size (), angmax);
  // 2- Perform third subdivision (ie along third long edge)
  subdivideRing1 (xyz1, ind1, ind1_size, ind1.size (), angmax);
}

void glGrib::Subdivide::append (std::vector<float> & lonlat, std::vector<unsigned int> & ind,
                               int points_offset, int triangles_offset)
{
  // Append new points
  for (size_t i = xyz1_size; i < xyz1.size (); i++)
    {   
      glm::vec2 ll = xyz2lonlat (xyz1[i]);
      int j = points_offset + i - xyz1_size;
      lonlat[2*j+0] = ll.x; lonlat[2*j+1] = ll.y;
    }   
 
  // Append new triangles
  for (size_t i = ind1_size; i < ind1.size (); i++)
    {   
      int j = triangles_offset + i - ind1_size;
      // Two cases 
      if (static_cast<int>(ind1[i]) < xyz1_size)     // This vertex already existed
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

