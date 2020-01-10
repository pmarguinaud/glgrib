#include "glgrib_test.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <omp.h>

#include <glm/gtc/type_ptr.hpp>
#include "glgrib_eigen3.h"


static
float getAngle (const glm::vec3 & xyz1, const glm::vec3 & xyz2) 
{
  return acos (std::max (-1.0f, 
               std::min (+1.0f, 
               glm::dot (xyz1, xyz2))));
}


void glgrib_test::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::TEST);
  program->use (); 

  view.setMVP (program);

//glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);

//glEnable (GL_CULL_FACE);

  view.delMVP (program);

}

void glgrib_test::clear ()
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

class node_t
{
public:

  node_t (int _rank, int _index) : rank (_rank), index (_index) {}

  std::string asString () const
  {
    std::string angle = "            ";
    
    return "node{" 
               + (prev ? std::to_string (prev->rank) : "NULL") + ", "
               + std::to_string (rank) + ", " 
               + (next ? std::to_string (next->rank) : "NULL") 
               + "}";
  }

  void setNext (node_t * _next) 
  {
    next = _next;
    dirty = true;
  }
  void setPrev (node_t * _prev) 
  {
    prev = _prev;
    dirty = true;
  }
  void setPrevNext (node_t * _prev, node_t * _next)
  {
    prev = _prev; next = _next; dirty = true;
  }
  node_t * getNext () const
  {
    return next;
  }
  node_t * getPrev () const
  {
    return prev;
  }
  int getRank () const
  {
    return rank;
  }
  int getIndex () const
  {
    return index;
  }

  float getAngle (const std::vector<glm::vec3> & xyz) const 
  {
    if (dirty)
      {
        int numberOfPoints1 = xyz.size () - 1;
        int j = index;
        int i = prev->index;
        int k = next->index;
        glm::vec3 ji = glm::cross (xyz[j], xyz[i] - xyz[j]);
        glm::vec3 jk = glm::cross (xyz[j], xyz[k] - xyz[j]);
        float X = glm::dot (ji, jk);
        float Y = glm::dot (xyz[j], glm::cross (ji, jk));
        ang = atan2 (Y, X);
        dirty = false;
      }
    return ang;
  }

  const glm::vec3 & getXYZ (const std::vector<glm::vec3> & xyz) const
  {
    return xyz[index];
  }

  float getLon (const std::vector<float> & lonlat) const
  {
    return lonlat[2*index+0];
  }

  float getLat (const std::vector<float> & lonlat) const
  {
    return lonlat[2*index+1];
  }

  float getX (const std::vector<float> & xy) const
  {
    return xy[2*index+0];
  }

  float getY (const std::vector<float> & xy) const
  {
    return xy[2*index+1];
  }

  bool inTriangle (const glm::vec3 & p, const std::vector<glm::vec3> & xyz) const
  {
    const glm::vec3 t[3] = {xyz[prev->index], xyz[index], xyz[next->index]};
  
    float S;
    bool o = true;
  
    for (int i = 0; i < 3; i++)
      {
        int j = (i + 1) % 3;
        const glm::vec3 u = glm::cross (t[i], t[j] - t[i]);
        float s = glm::dot (p - t[i], u);
        if (o)
          {
            if (s != 0.0f)
              {
                S = s; 
                o = false;
              }
          }
        else if (s * S < 0.0f)
          {
            return false;
          }
      }
  
    return true;
  }

  void cut (node_t ** x, node_t ** list)
  {
    node_t * n = next, * p = prev;
    n->setPrev (prev);
    p->setNext (next);

    *x = next;

    next = prev = NULL; 

    if (this == *list)
      *list = *x;
  }

  bool linked () const
  {
    return (next != NULL) || (prev != NULL);
  }

  int count () const
  {
    int c = 0;
    for (const node_t * n = this; ; )
      {
        c++;
        n = n->next;
        if (n == this)
          break;
      }
    return c;
  }


private:
  int rank = -1, index = -1;
  node_t * prev = NULL, * next = NULL;

  mutable float ang = 0.0f;
  mutable bool dirty = true;
  
};

static float in02pi (float x)
{
  while (x < 0.0f ) x += twopi;
  while (x > twopi) x -= twopi;
  return x;
}

static void xint (float & x1, float & x2)
{
  x1 = in02pi (x1);
  x2 = in02pi (x2);

  if (fabs (x1 - x2) > pi)
    {
      if (x1 < x2)
        std::swap (x1, x2);
    }
  else
    {
      if (x1 > x2)
        std::swap (x1, x2);
    }
}

class inter_t
{
public:
  inter_t () {}
  inter_t (float _xmin, float _xmax) : xmin (_xmin), xmax (_xmax) {}
  bool contains (float x) const
  {
    if (full)
      return true;
    if (xmin <= xmax)
      return (xmin <= x) && (x <= xmax);
    else
      return (x <= xmax) || (xmin <= x);
  }
  void expand (const inter_t & inter)
  {

    if (full)
      return;

    if (inter.full)
      {
        full = true;
        return;
      }

    bool cmin = contains (inter.xmin);
    bool cmax = contains (inter.xmax);


    if (cmin && cmax)
      {
        if (xmin < xmax)
          { 
            if (inter.xmin < inter.xmax)
              return;
            else
              full = true;
          }
        else
          {
            bool cmin1 = inter.contains (xmin);
            bool cmax1 = inter.contains (xmax);
            if (cmin1 && cmax1)
              full = true;
            else
              return;
          }
      }
    else if (cmin && (! cmax))
      {
        xmax = inter.xmax;
      }
    else if ((! cmin) && cmax)
      {
        xmin = inter.xmin;
      }
    else if ((! cmin) && (! cmax))
      {
        if (in02pi (xmin - inter.xmax) < in02pi (inter.xmin - xmax))
          xmin = inter.xmin;
        else
          xmax = inter.xmax;
      }

  }

  float xmin, xmax;
  bool full = false;
};
        

static
void getLonRange (const std::vector<node_t> & nodevec, 
                  const std::vector<float> & lonlat,
                  float * lonmin, float * lonmax)
{
  inter_t minmax;
  bool init = false;

  for (int i = 0; i < nodevec.size (); i++)
    {

      const node_t * n1 = &nodevec[i];
      const node_t * n2 = n1->getNext ();

      float x1 = n1->getX (lonlat);
      float x2 = n2->getX (lonlat);

      xint (x1, x2);

      if (x1 == x2) 
        continue;

      if (! init)
        {
          minmax.xmin = x1;
          minmax.xmax = x2;
	  init = true;
        }
      else
        {
          minmax.expand (inter_t (x1, x2));
        }

    }

  *lonmin = minmax.xmin;
  *lonmax = minmax.xmax;

}

std::ostream & operator<< (std::ostream& os, const node_t & n)
{
  os << n.asString ();
  return os;
}

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

static
void getLatRange (const glm::vec2 & lonlat1, const glm::vec2 & lonlat2, 
                  float * latmin, float * latmax)
{
  const glm::vec3 p1 = lonlat2xyz (lonlat1);
  const glm::vec3 p2 = lonlat2xyz (lonlat2);
 
  float zmin, zmax;

  zmin = std::min (p1.z, p2.z);
  zmax = std::max (p1.z, p2.z);

  glm::vec3 p = glm::cross (p1, p2);

  float np = glm::length (p);

  if (np == 0.0f)
    {
      if (glm::dot (p1, p2) < 0.0f)
        zmax = 1.0f;
      return;
    }

  p = p / np;
  glm::vec3 q1 = glm::cross (p, p1);

  float theta_op1 = atan2 (q1.z, p1.z), 
        theta_op2 = theta_op1 + pi,
        theta_max = atan2 (glm::dot (p2, q1), glm::dot (p2, p1));

  if ((0 <= theta_op1) && (theta_op1 <= theta_max))
    {
      float z = p1.z * cos (theta_op1) + q1.z * sin (theta_op1);
      zmin = std::min (zmin, z); zmax = std::max (zmax, z);
    }

  if ((0 <= theta_op2) && (theta_op2 <= theta_max))
    {
      float z = p1.z * cos (theta_op2) + q1.z * sin (theta_op2);
      zmin = std::min (zmin, z); zmax = std::max (zmax, z);
    }

  *latmin = asin (zmin);
  *latmax = asin (zmax);
}

static
void getLatRange (const std::vector<node_t> & nodevec, 
                  const std::vector<float> & lonlat,
                  float * latmin, float * latmax)
{
  *latmax = *latmin = lonlat[2*0+1];
  for (int i = 0; i < nodevec.size (); i++)
    {
      float lat = lonlat[2*i+1];
      *latmin = std::min (lat, *latmin);
      *latmax = std::max (lat, *latmax);
    }

  *latmax = *latmin = lonlat[2*0+1];
  for (const node_t * n = &nodevec[0]; ; )
    {

 
      float ri = n            ->getIndex ();
      float rj = n->getNext ()->getIndex ();

      float llatmin, llatmax;

      getLatRange (glm::vec2 (lonlat[2*ri+0], lonlat[2*ri+1]),
                   glm::vec2 (lonlat[2*rj+0], lonlat[2*rj+1]),
                   &llatmin, &llatmax);

      *latmin = std::min (*latmin, llatmin);
      *latmax = std::max (*latmax, llatmax);

      n = n->getNext ();
      if (n == &nodevec[0])
        break;
    }

if(0)
  std::cout << " latmin, latmax = " << rad2deg * *latmin 
            << ", " << rad2deg * *latmax << std::endl;

}


class xy2node_t
{
public:
  xy2node_t (const std::vector<node_t> & nodevec,
             const std::vector<float> & xy, 
             const std::vector<glm::vec3> & xyz,
	     bool _openmp)
  {
    openmp = _openmp;

    float lonmin, lonmax, latmin, latmax;

    getLonRange (nodevec, xy, &lonmin, &lonmax);
    getLatRange (nodevec, xy, &latmin, &latmax);

if(0)
    printf (" lon = %12.2f .. %12.2f, lat = %12.2f .. %12.2f\n",
            rad2deg * lonmin, rad2deg * lonmax, rad2deg * latmin, rad2deg * latmax);

    float ddy = std::max ((latmax - latmin) / ny, 0.1f * deg2rad);

    ymin = std::max (-pi, latmin - ddy);
    ymax = std::min (+pi, latmax + ddy);
    dy = (ymax - ymin) / ny;


    xmin = lonmin;
    xmax = lonmax;

    while (xmin > xmax)
      xmin -= twopi;

    float ddx = std::max ((xmax - xmin) / nx, 0.1f * deg2rad);

if(0)
    printf (" ddx = %12.2e\n", ddx * rad2deg);

    xmin = xmin - ddx;
    xmax = xmax + ddx;

    dx = (xmax - xmin) / nx;

    off.resize (nx * ny);
    len.resize (nx * ny);

    for (int k = 0; k < len.size (); k++)
      len[k] = 0;

    for (int i = 0; i < nodevec.size (); i++)
      {
        if (nodevec[i].getAngle (xyz) < 0.0f)
          {
            int ind = xy2ind (xy[2*i+0], xy[2*i+1]);
            len[ind]++;
          }
      }

    off[0] = 0;
    for (int k = 1; k < off.size (); k++)
      off[k] = off[k-1] + len[k-1];

    nodes.resize (off.back () + len.back ());

    std::vector<int> cur = off;
    
    for (int i = 0; i < nodevec.size (); i++)
      {
        if (nodevec[i].getAngle (xyz) < 0.0f)
          {
            int ind = xy2ind (xy[2*i+0], xy[2*i+1]);
            nodes[cur[ind]] = &nodevec[i];
            cur[ind]++;
          }
      }
  }

  void update (const std::vector<glm::vec3> & xyz)
  {
#pragma omp parallel for if (openmp)
    for (int i = 0; i < len.size (); i++)
      for (int j = off[i]; j < off[i] + len[i]; j++)
        if ((! nodes[j]->linked ()) || (nodes[j]->getAngle (xyz) >= 0.0f))
          {
            int k = off[i]+len[i]-1;
            std::swap (nodes[j], nodes[k]);
            len[i]--;
            j--;
          }
  }

  int xy2ind (float x, float y) const
  {
    while (x < xmin) x += twopi;
    while (x > xmax) x -= twopi;
    int ix = nx * (x - xmin) / (xmax - xmin);
    int iy = ny * (y - ymin) / (ymax - ymin);

    if ((ix < 0) || (nx <= ix))
      abort ();
    if ((iy < 0) || (ny <= iy))
      abort ();

    int ind = ix + iy * nx;
    return ind;
  }

  int xy2ind (int ix, int iy) const
  {
    while (ix <  0) ix += nx;
    while (ix > nx) ix -= nx;

    if ((ix < 0) || (nx <= ix))
      abort ();
    if ((iy < 0) || (ny <= iy))
      abort ();

    int ind = ix + iy * nx;
    return ind;
  }

  class iterator 
  {
  public:
    iterator (float _ymin, float _ymax, float _xmin, float _xmax, const xy2node_t * _ll2n) 
    : ymin (_ymin), ymax (_ymax), xmin (_xmin), xmax (_xmax), ll2n (_ll2n)
    {

      while (xmin < ll2n->xmin) xmin += twopi;
      while (xmin > ll2n->xmax) xmin -= twopi;

      if ((xmin < ll2n->xmin) || (ll2n->xmax < xmin))
        abort ();

      while (xmax < ll2n->xmin) xmax += twopi;
      while (xmax > ll2n->xmax) xmax -= twopi;

      if ((xmax < ll2n->xmin) || (ll2n->xmax < xmax))
        abort ();

static 
bool dbg = false;

      iymin = floor ((ymin - ll2n->ymin) / ll2n->dy); 
      iymax = floor ((ymax - ll2n->ymin) / ll2n->dy);
      ixmin = floor ((xmin - ll2n->xmin) / ll2n->dx); 
      ixmax = floor ((xmax - ll2n->xmin) / ll2n->dx);

      ny = iymax - iymin + 1;
      nx = ixmax - ixmin + 1;

      if ((ixmin < 0) || (ll2n->nx <= ixmax))
        abort ();

      if ((iymin < 0) || (ll2n->ny <= iymax))
        abort ();


if(dbg){
      std::cout << " iymin = " << iymin << " iymax = " << iymax << std::endl;
      std::cout << " ixmin = " << ixmin << " ixmax = " << ixmax << std::endl;
}

      xmin = ixmin * ll2n->dx         ; xmax = ixmax * ll2n->dx         ;
      ymin = iymin * ll2n->dy + ll2n->ymin; ymax = iymax * ll2n->dy + ll2n->ymin;

if(dbg){
      std::cout << "  ymin = " << rad2deg * ymin << "  ymax = " << rad2deg * ymax << std::endl;
      std::cout << "  xmin = " << rad2deg * xmin << "  xmax = " << rad2deg * xmax << std::endl;
}

dbg = false;

      this->seek (0);
    }
    void seek (int whence)
    {
      if (whence == 0)
        {
          ind = ll2n->xy2ind (ixmin, iymin);
          cur = 0;
          iy = 0; ix = 0;
          if (ll2n->len[ind] == 0)
            operator++ ();
        }
      else
        {
          cur = 0; iy = ny; ix = 0;
        }
    }

    iterator & operator++ ()
    {
       cur++;
       while (cur >= ll2n->len[ind])
         {
           cur = 0;
           ix++;
           if (ix >= nx)
             {
               ix = 0;
               iy++;
               if (iy == ny)
                 break;
             }
           ind = ll2n->xy2ind (ixmin + ix, iymin + iy);
         }
       return *this;
    }


    const node_t * const & operator* ()
    {
      return ll2n->nodes[ll2n->off[ind]+cur];
    }

    bool operator!= (const iterator & rhs) const
    {
      return ! operator== (rhs);
    }
    bool operator== (const iterator & rhs) const
    {
      return (ll2n == rhs.ll2n) && (ix == rhs.ix) && 
             (iy == rhs.iy) && (cur == rhs.cur);
    }

    std::string asString () const
    {
      int ix1 = ind % ll2n->nx;
      int iy1 = ind / ll2n->nx;
      float x1 = ix1 * ll2n->dx,              x2 = x1 + ll2n->dx;
      float y1 = iy1 * ll2n->dy + ll2n->ymin, y2 = y1 + ll2n->dy;
      
      return std::string ("iterator{") +
             "ix  =" + std::to_string (ix) + "/" + std::to_string (nx)           + ", " +
             "iy  =" + std::to_string (iy) + "/" + std::to_string (ny)           + ", " +
             "cur =" + std::to_string (cur)  + "/" + std::to_string (ll2n->len[ind]) + ", " +
             "ind =" + std::to_string (ind)  +                                         ", " +
             "off =" + std::to_string (ll2n->off[ind])     + ", " +
             "len =" + std::to_string (ll2n->len[ind])     + ", " +
             "    =" + std::to_string (ll2n->off[ind]+cur) +
             "[" + std::to_string (rad2deg * x1) + ".." + std::to_string (rad2deg * x2) + "]" +
             "[" + std::to_string (rad2deg * y1) + ".." + std::to_string (rad2deg * y2) + "]" +
             "}";
    }

  private:
    int ind; // Current index in ll2n
    int cur; // Current index in nodes
    int iy, ix;
    int ny, nx;
    float  ymin,  ymax,  xmin,  xmax;
    int   iymin, iymax, ixmin, ixmax;
    const xy2node_t * ll2n = NULL;
  };

  std::string asString (const std::vector<glm::vec3> & xyz) const
  {
    std::string t;
    for (int i = 0; i < len.size (); i++)
      for (int j = off[i]; j < off[i] + len[i]; j++)
        {
          t += std::to_string (nodes[j]->getRank ()) + " " + 
               std::to_string (nodes[j]->getAngle (xyz) * rad2deg) + "\n";
        }
    return t;
  }

  iterator begin (const node_t & n, const std::vector<float> & xy, bool dbg = false) const
  {
    const node_t * nn[3] = {n.getPrev (), &n, n.getNext ()};

    float ymin = +pi, ymax = -pi;

    for (int i = 0; i < 3; i++)
      {
        int j = (i + 1) % 3;
        int ri = nn[i]->getIndex (); 
        int rj = nn[j]->getIndex ();
        float yymin, yymax;
        getLatRange (glm::vec2 (xy[2*ri+0], xy[2*ri+1]), 
                     glm::vec2 (xy[2*rj+0], xy[2*rj+1]), 
                     &yymin, &yymax);
        ymin = std::min (ymin, yymin);
        ymax = std::max (ymax, yymax);
      }

    float x0 = n.            getX (xy);
    float x1 = n.getNext ()->getX (xy);
    float x2 = n.getPrev ()->getX (xy);

    inter_t int01, int02;

    {
      float x0 = n.            getX (xy);
      float x1 = n.getNext ()->getX (xy);
      xint (x0, x1);
      int01 = inter_t (x0, x1);
    }

    {
      float x0 = n.            getX (xy);
      float x2 = n.getPrev ()->getX (xy);
      xint (x0, x2);
      int02 = inter_t (x0, x2);
    }

    int01.expand (int02);

    float xmin = int01.xmin;
    float xmax = int01.xmax;

    while (xmax - xmin > twopi) xmax -= twopi;
    while (xmin - xmax > twopi) xmin -= twopi;

    return iterator (ymin, ymax, xmin, xmax, this);
  }

  iterator end (const node_t & n, const std::vector<float> & xy) const
  {
    iterator it = begin (n, xy);
    it.seek (1);
    return it;
  }

  int size () const 
  {
    int s = 0;
    for (int i = 0; i < len.size (); i++)
      s += len[i];
    return s;
  }


private:
  friend class iterator;
  int nx = 360, ny = 180;
  std::vector<int> off, len;
  std::vector<const node_t*> nodes;
  float dx, dy;
  float ymin = -halfpi, ymax = +halfpi;
  float xmin = 0.0f,    xmax = twopi;
  bool openmp = true;
};


std::ostream & operator<< (std::ostream& os, const xy2node_t::iterator & it)
{
  os << it.asString ();
  return os;
}



static 
void earCut (node_t ** nodelist,  
             const std::vector<glm::vec3> & xyz,
             const std::vector<float> & xy,
             const xy2node_t & ll2n,
             std::vector<unsigned int> * ind,
	     bool openmp)
{

  std::vector<node_t*> b, e;
  int count = (*nodelist)->count ();
  int pools;

  if ((count > 100) && openmp)
    {

      for (int i = 0; i < 10; i++)
        (*nodelist) = (*nodelist)->getNext ();

      pools = 8;
      int chunk = count / pools;
      int i = 0;
      for (node_t * n = *nodelist; n != (*nodelist)->getPrev (); n = n->getNext ())
        {
          if (i % chunk == 0)
            b.push_back (n);
     
          if ((i + 3) % chunk == 0)
            e.push_back (n);
     
          i++;
        }
    }
  else
    {
      pools = 1;
      b.push_back (*nodelist);
      e.push_back ((*nodelist)->getPrev ());
    }

  if (0)
  for (int c = 0; c < pools; c++)
    {
      std::cout << c << " " << b[c]->getRank () << " ... " 
                << e[c]->getRank () << std::endl;
    }

  std::vector<unsigned int> ind1[pools];

#pragma omp parallel for if (openmp)
  for (int c = 0; c < pools; c++)
    {
      std::vector<unsigned int> * ind = &ind1[c];
      for (node_t * n = b[c]; n != e[c]; n = n->getNext ())
        {
          float ang = n->getAngle (xyz);

          if ((0.0f < ang) && (ang < M_PI))
            {
      
              bool intri = false;

              xy2node_t::iterator it = ll2n.begin (*n, xy);
              xy2node_t::iterator it1 = ll2n.end (*n, xy);
              for ( ; it != it1; ++it)
                {
                  const node_t * n1 = *it;
                  const glm::vec3 & p = n1->getXYZ (xyz);

                  if ((n == n1) || (n->getNext () == n1) || (n->getPrev () == n1))
                    continue;

                  if (n->inTriangle (p, xyz))
                    {
                      intri = true;
                      break;
                    }
                }

              if (! intri)
                {
                  ind->push_back (n            ->getRank ());
                  ind->push_back (n->getPrev ()->getRank ());
                  ind->push_back (n->getNext ()->getRank ());
                  n->cut (&n, nodelist);
                  if (n == e[c])
                    break;
                }

            }

        }
    }

  int size = ind->size ();
 
  for (int c = 0; c < pools; c++)
    size += ind1[c].size ();

  int offset[pools];
  offset[0] = ind->size ();
  for (int c = 1; c < pools; c++)
    offset[c] = offset[c-1] + ind1[c-1].size ();

  ind->resize (size);

#pragma omp parallel for if (openmp)
  for (int c = 0; c < pools; c++)
    for (int i = 0; i < ind1[c].size (); i++)
      (*ind)[offset[c]+i] = ind1[c][i];

}

static 
glm::mat3 getRotMat (const std::vector<glm::vec3> & xyz, bool openmp)
{
  // Average point

  glm::dvec3 dxyzm (0.0f, 0.0f, 0.0f);

  std::vector<float> w;
  float W = 0.0f;

  w.resize (xyz.size ());

#pragma omp parallel for if (openmp)
  for (int i = 0; i < xyz.size (); i++)
    {
      int j = i != xyz.size () - 1 ? i + 1 : 0;
      w[i] = getAngle (xyz[i], xyz[j]);
    }

  for (int i = 0; i < xyz.size (); i++)
    W += w[i];

  if (W == 0.0f)
    {
      W = xyz.size ();
      for (int i = 0; i < xyz.size (); i++)
        w[i] = 1.0f;
    }

#pragma omp parallel for if (openmp)
  for (int i = 0; i < xyz.size (); i++)
    w[i] = w[i] / W;


  for (int i = 0; i < xyz.size (); i++)
    dxyzm += w[i] * xyz[i];

  // Covariance
  glm::dmat3 A;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    A[i][j] = 0.0f;

#pragma omp parallel for collapse (2) if (openmp)
  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
  for (int k = 0; k < xyz.size (); k++)
    A[i][j] += w[k] * (xyz[k][i] - dxyzm[i]) * (xyz[k][j] - dxyzm[j]);



#ifdef UNDEF

  std::cout << " det (A) = " << glm::determinant (A) << std::endl;

  printf (" A = \n");
  for (int i = 0; i < 3; i++) 
    {
      for (int j = 0; j < 3; j++) 
        printf (" %12.5f", A[i][j]);
      printf ("\n");
    }
  
  printf ("--------\n");
  for (int i = 0; i < 3; i++) 
    {
      glm::vec3 u (0.0f, 0.0f, 0.0f);
      u[i] = 1.0f;
      glm::vec3 v = A * u;
      for (int j = 0; j < 3; j++) 
        printf (" %12.5f", v[j]);
      printf ("\n");
    }
  
    
  printf ("--------\n");
  for (int i = 0; i < 3; i++) 
    {
      glm::vec3 u (1.0f, 1.0f, 1.0f);
      u[i] = 0.0f;
      glm::vec3 v = A * u;
      for (int j = 0; j < 3; j++) 
        printf (" %12.5f", v[j]);
      printf ("\n");
    }
 
#endif
    
  glm::dvec3 e;
  glm::dmat3 Q;

  // Get eigenvectors & eigenvalues

  glgrib_diag (A, &Q, &e);

#ifdef UNDEF

  glm::dmat3 B = glm::transpose (Q) * A * Q;

  printf (" B = \n");
  for (int i = 0; i < 3; i++) 
    {
      for (int j = 0; j < 3; j++) 
        printf (" %12.5f", B[i][j]);
      printf ("\n");
    }
  


  for (int i = 0; i < 3; i++)
    {
      glm::vec2 lonlat = xyz2lonlat (Q[i]);
      printf (" > %12.2f %12.2f\n", rad2deg * lonlat.x, rad2deg * lonlat.y);
    }


  printf (" %12.2e %12.2e %12.2e\n", e[0], e[1], e[2]);

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 3; j++)
    {
      printf (" %8d, %8d -> %12.2f\n", i, j, glm::dot (Q[i], Q[j]));
    }
  for (int i = 0; i < 3; i++)
    {
      glm::dvec3 u (Q[i][0], Q[i][1], Q[i][2]);
      glm::dvec3 q = A * u;
      printf (" u   = %12.2e %12.2e %12.2e\n", u[0], u[1], u[2]);
      printf (" q   = %12.2e %12.2e %12.2e\n", q[0], q[1], q[2]);
      printf (" q/u = %12.2e %12.2e %12.2e\n", q[0]/u[0], q[1]/u[1], q[2]/u[2]);
    }
  for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
        printf (" %12.2e", Q[i][j]);
      printf ("\n");
    }
#endif


  Q[1] = Q[2]; Q[2] = glm::cross (Q[0], Q[1]);


#ifdef UNDEF
  const char * n = "XYZ";
  for (int i = 0; i < 3; i++)
    {
      glm::vec2 lonlat = xyz2lonlat (Q[i]);
      printf ("%c > %12.2f %12.2f\n", n[i], rad2deg * lonlat.x, rad2deg * lonlat.y);
    }
#endif

  return glm::mat3 (Q);
}



static 
void processRing (const std::vector<float> & lonlat1, 
                  int rank1, int rank2, 
                  std::vector<unsigned int> * ind,
		  bool openmp)
{
  std::vector<glm::vec3> xyz1;

  int numberOfPoints1 = rank2-rank1;

if(0)
  printf (" numberOfPoints1 = %d\n", numberOfPoints1);

  xyz1.resize (numberOfPoints1);

#pragma omp parallel for if (openmp)
  for (int i = 0; i < numberOfPoints1; i++)
    {
      int j = i + rank1;
      float lon = lonlat1[2*j+0], lat = lonlat1[2*j+1];
      float coslon = cos (lon), sinlon = sin (lon);
      float coslat = cos (lat), sinlat = sin (lat);
      xyz1[i] = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
    }

if(0)
  for (int i = 0; i < numberOfPoints1; i++)
     printf (" %8d | %8d | %12.2e %12.2e %12.2e | %18.6f %18.6f\n", 
             i, i+rank1, xyz1[i].x, xyz1[i].y, xyz1[i].z, 
	     rad2deg * lonlat1[2*(i+rank1)+0], rad2deg * lonlat1[2*(i+rank1)+1]);


  glm::mat3 R = getRotMat (xyz1, openmp);

if(0)
  for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
        printf (" %12.2e", R[i][j]);
      printf ("\n");
    }



  std::vector<glm::vec3> xyz2;
  std::vector<float> lonlat2;

  xyz2.resize (xyz1.size ());
  lonlat2.resize (2 * xyz1.size ());

#pragma omp parallel for if (openmp)
  for (int i = 0; i < xyz1.size (); i++)
    {
      xyz2[i] = R * xyz1[i];
      lonlat2[2*i+0] = atan2 (xyz2[i].y, xyz2[i].x); 
      lonlat2[2*i+1] = asin (xyz2[i].z);
    }

if(0)
  for (int i = 0; i < xyz1.size (); i++)
     printf (" %8d | %12.2e %12.2e %12.2e | %18.6f %18.6f\n", 
             i, xyz2[i].x, xyz2[i].y, xyz2[i].z, 
	     rad2deg * lonlat2[2*i+0], rad2deg * lonlat2[2*i+1]);

  std::vector<glm::vec3> & xyz = xyz2;
  std::vector<float> & lonlat = lonlat2;

  std::vector<node_t> nodevec;

  nodevec.reserve (numberOfPoints1);

  for (int j = 0; j < numberOfPoints1; j++)
    nodevec.push_back (node_t (rank1+j, j));

#pragma omp parallel for if (openmp)
  for (int j = 0; j < numberOfPoints1; j++)
    {
      int i = j == 0 ? numberOfPoints1-1 : j-1;
      int k = j == numberOfPoints1-1 ? 0 : j+1;
      nodevec[j].setPrevNext (&nodevec[i], &nodevec[k]);
      nodevec[j].getAngle (xyz);
    }

  xy2node_t ll2n (nodevec, lonlat, xyz, openmp);

  node_t * nodelist = &nodevec[0];

  for (int i = 0, k = -1; ; i++)
    {
//    printf (" %8d, nodelist = %8d, ll2n = %8d\n", i, nodelist->count (), ll2n.size ());
      earCut (&nodelist, xyz, lonlat, ll2n, ind, openmp);
      ll2n.update (xyz);

      int count = nodelist->count ();
      if (count == k)
        break;
      k = count;
    }
//printf (" %8s  nodelist = %8d, ll2n = %8d\n", "", nodelist->count (), ll2n.size ());

}


void glgrib_test::setup (const glgrib_options_test & o)
{
  opts = o;

  int numberOfPoints;
  glgrib_options_lines lopts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat1;
  std::vector<unsigned int> indl;
  
  lopts.path = opts.path; lopts.selector = opts.selector;
  glgrib_shapelib::read (lopts, &numberOfPoints, &numberOfLines, &lonlat1, 
                         &indl, lopts.selector);


  std::vector<unsigned int> ind;

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

  {
    FILE * fp = fopen ("length.txt", "w");
    for (int i = 0; i < length.size (); i++)
      fprintf (fp, "%d\n", length[ord[i]]);
    fclose (fp);
  }

  std::cout << ord.size () << std::endl;

  int ind_size = 0;
  for (int k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
      if (length[j] > 2)
        {
          ind_size += 3 * (length[j] - 2);
        }
    }

  int k = 0;
  for (k = 0; k < ord.size (); k++)
    {
      int j = ord[k];
//    if (length[j] < 300)
//      break;
      int size = ind.size ();
      if (length[j] > 2)
        {
          processRing (lonlat1, offset[j], offset[j]+length[j], &ind, true);
          if (ind.size () - size != 3 * (length[j] - 2))
          printf ("%8d, %8d | %8d %8lu %8d\n", k, j, length[j], 
                  ind.size () - size, 3 * (length[j] - 2));
        }
    }

  std::cout << " ind_size    = " << ind_size    << std::endl;
  std::cout << " ind.size () = " << ind.size () << std::endl;

//exit (0);


  numberOfTriangles = ind.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (lonlat1.size () * sizeof (lonlat1[0]), 
                                               lonlat1.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (ind[0]), 
                                                ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

