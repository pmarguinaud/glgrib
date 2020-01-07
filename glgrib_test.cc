#include "glgrib_test.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <omp.h>


void glgrib_test::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  glgrib_program * program = glgrib_program::load (glgrib_program::TEST);
  program->use (); 

  view.setMVP (program);

  glDisable (GL_CULL_FACE);

  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, NULL);
  glBindVertexArray (0);

  glEnable (GL_CULL_FACE);
  

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

  node_t (int _rank) : rank (_rank) {}

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

  float getAngle (const std::vector<glm::vec3> & xyz) const 
  {
    if (dirty)
      {
        int numberOfPoints1 = xyz.size () - 1;
        int j = rank;
        int i = prev->rank;
        int k = next->rank;
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
    return xyz[rank];
  }

  float getLon (const std::vector<float> & lonlat) const
  {
    return lonlat[2*rank+0];
  }

  float getLat (const std::vector<float> & lonlat) const
  {
    return lonlat[2*rank+1];
  }

  float getX (const std::vector<float> & xy) const
  {
    return xy[2*rank+0];
  }

  float getY (const std::vector<float> & xy) const
  {
    return xy[2*rank+1];
  }

  bool inTriangle (const glm::vec3 & p, const std::vector<glm::vec3> & xyz) const
  {
    const glm::vec3 t[3] = {xyz[prev->rank], xyz[rank], xyz[next->rank]};
  
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
  int rank = -1;
  node_t * prev = NULL, * next = NULL;

  mutable float ang = 0.0f;
  mutable bool dirty = true;
  
};

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
void getLonLatRange (const glm::vec2 & lonlat1, const glm::vec2 & lonlat2, 
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


class xy2node_t
{
public:
  void init (const std::vector<node_t> & nodevec,
             const std::vector<float> & xy, 
             const std::vector<glm::vec3> & xyz)
  {

    dx = (xmax - xmin) / nx;
    dy = (ymax - ymin) / ny;

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
//#pragma omp parallel for
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
    int ix = nx * x / (xmax - xmin);
    int iy = ny * (y - ymin) / (ymax - ymin);
    int ind = ix + iy * nx;
    if (ind >= off.size ())
      abort ();
    return ind;
  }

  int xy2ind (int ix, int iy) const
  {
    while (ix <  0) ix += nx;
    while (ix > nx) ix -= nx;
    int ind = ix + iy * nx;
    if (ind >= off.size ())
      abort ();
    return ind;
  }

  class iterator 
  {
  public:
    iterator (float _ymin, float _ymax, float _xmin, float _xmax, const xy2node_t * _ll2n) 
    : ymin (_ymin), ymax (_ymax), xmin (_xmin), xmax (_xmax), ll2n (_ll2n)
    {

static 
bool dbg = false;

      iymin = floor ((ymin - ll2n->ymin) / ll2n->dy), iymax = floor ((ymax - ll2n->ymin) / ll2n->dy);
      ixmin = floor ((xmin - ll2n->xmin) / ll2n->dx), ixmax = floor ((xmax - ll2n->xmin) / ll2n->dx);

      ny = iymax - iymin + 1;
      nx = ixmax - ixmin + 1;

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
           ind = ll2n->xy2ind (ixmin + ix, 
                                   iymin + iy);
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

  iterator begin (const node_t & n, const std::vector<float> & xy) const
  {
    float y0 = n.            getY (xy);
    float y1 = n.getNext ()->getY (xy);
    float y2 = n.getPrev ()->getY (xy);

    float ymin = std::min (y0, std::min (y1, y2));
    float ymax = std::max (y0, std::max (y1, y2));

    float x0 = n.            getX (xy);
    float x1 = n.getNext ()->getX (xy);
    float x2 = n.getPrev ()->getX (xy);


    while (x1 - x0 > +twopi) x1 -= twopi;
    while (x1 - x0 < -twopi) x1 += twopi;
    while (x2 - x0 > +twopi) x2 -= twopi;
    while (x2 - x0 < -twopi) x2 += twopi;

    float xmin = std::min (x0, std::min (x1, x2));
    float xmax = std::max (x0, std::max (x1, x2));

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
  const float ymin = -halfpi, ymax = +halfpi;
  const float xmin = 0.0f,    xmax = twopi;
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
             std::vector<node_t*> * ain)
{

  node_t * last = (*nodelist)->getPrev ();

  for (node_t * n = *nodelist; ; )
    {
      float ang = n->getAngle (xyz);

      if ((0.0f < ang) && (ang < M_PI))
        {
  
#ifndef UNDEF
          volatile 
          int intri = 0;

//#pragma omp parallel
          {
          int nt = omp_get_num_threads ();
          int it = omp_get_thread_num ();

          int i1 = (ain->size () * (it + 0)) / nt;
          int i2 = (ain->size () * (it + 1)) / nt;

          for (int i = i1; i < i2; i++)
            {
              node_t * n1 = (*ain)[i];
              if ((n1 != n) && n1->linked () && (n1 != n->getNext ()) && 
                  (n1 != n->getPrev ()))
                {
                  const glm::vec3 & p = n1->getXYZ (xyz);
                  if (n->inTriangle (p, xyz))
                    {
//#pragma omp atomic
                      intri++;
                    }
                }
              if (intri)
                break;
            }
          }
#else

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
#endif

          if (! intri)
            {
              
              ind->push_back (n->getPrev ()->getRank ());
              ind->push_back (n            ->getRank ());
              ind->push_back (n->getNext ()->getRank ());

              
              n->cut (&n, nodelist);

            }

        }


      n = n->getNext ();
      if (n == last)
        break;
    }


  for (int i = 0; i < ain->size (); i++)
    {
      node_t * n = (*ain)[i];
      if ((! n->linked ()) || (n->getAngle (xyz) >= 0.0f))
        {
          std::swap ((*ain)[i], ain->back ());
          ain->resize (ain->size () - 1);
          i--;
        }
    }

   
}

static void dump (node_t * nodelist, const std::vector<node_t*> & ain,
                  const xy2node_t & ll2n, const std::vector<glm::vec3> & xyz,
                  const std::vector<float> & xy, const std::string & prefix)
{
{
  std::string filename = prefix + ".ain.txt";
  FILE * fp = fopen (filename.c_str (), "w");

  fprintf (fp, " %8lu \n", ain.size ());
  
  for (int i = 0; i < ain.size (); i++)
    fprintf (fp, " %d %f\n", ain[i]->getRank (), ain[i]->getAngle (xyz) * rad2deg);
  fprintf (fp, "\n");

  for (node_t * n = nodelist; ; )
    {
      std::vector<int> ii;

      for (int i = 0; i < ain.size (); i++)
        {
          const node_t * n1 = ain[i];
          const glm::vec3 & p = n1->getXYZ (xyz);
          if ((n == n1) || (n->getNext () == n1) || (n->getPrev () == n1))
            continue;
          if (n->inTriangle (p, xyz))
            ii.push_back (n1->getRank ());
        }
      if (ii.size ())
        {
          int rank = n->getRank ();
          sort (ii.begin (), ii.end ()); 
          fprintf (fp, "%8d | %12.2f %12.2f %12.2f :", rank,
                   rad2deg * n->getAngle (xyz), xy[2*rank+0] * rad2deg, xy[2*rank+1] * rad2deg);
          for (int i = 0; i < ii.size (); i++)
            fprintf (fp, " %8d", ii[i]);
          fprintf (fp, "\n");
        } 

      n = n->getNext ();
      if (n == nodelist)
        break;
    }

  fclose (fp);
}
{
  std::string filename = prefix + ".ll2.txt";
  FILE * fp = fopen (filename.c_str (), "w");
  fprintf (fp, " %8d \n", ll2n.size ());

  fprintf (fp, "%s\n", ll2n.asString (xyz).c_str ());

  for (node_t * n = nodelist; ; )
    {
      std::vector<int> ii;

bool dbg = n->getRank () == 297;

      xy2node_t::iterator it = ll2n.begin (*n, xy);
      xy2node_t::iterator it1 = ll2n.end (*n, xy);
      for ( ; it != it1; ++it)
        {
          const node_t * n1 = *it;
          const glm::vec3 & p = n1->getXYZ (xyz);
          if ((n == n1) || (n->getNext () == n1) || (n->getPrev () == n1))
            continue;
          if (n->inTriangle (p, xyz))
            ii.push_back (n1->getRank ());
        }
      if (ii.size ())
        {
          int rank = n->getRank ();
          sort (ii.begin (), ii.end ()); 
          fprintf (fp, "%8d | %12.2f %12.2f %12.2f :", rank,
                   rad2deg * n->getAngle (xyz), xy[2*rank+0] * rad2deg, xy[2*rank+1] * rad2deg);
          for (int i = 0; i < ii.size (); i++)
            fprintf (fp, " %8d", ii[i]);
          fprintf (fp, "\n");
        } 
      n = n->getNext ();
      if (n == nodelist)
        break;
    }
  fclose (fp);
}
}


static
void pr (const std::vector<float> & xy, 
         const std::vector<glm::vec3> & xyz,
         const xy2node_t & ll2n,
         int jj, node_t * nodelist)
{

  node_t * n;
  
  for (n = nodelist; ; )
    {
      n = n->getNext ();

      if (n->getRank () == jj)
        break;

      if (n == nodelist)
        return;
    }

{
  std::cout << jj << " -> " << *n << std::endl;

  xy2node_t::iterator it = ll2n.begin (*n, xy);
  xy2node_t::iterator it1 = ll2n.end (*n, xy);
  for ( ; it != it1; ++it)
    {
      const node_t * n = *it;
      int rank = n->getRank ();
      std::cout << it << " " << *n << " ";
      printf (" %12.2f %12.2f %12.2f", 
              rad2deg * xy[2*rank+0], 
              rad2deg * xy[2*rank+1], 
              rad2deg * n->getAngle (xyz));
      std::cout << std::endl;
    }

}

{
  FILE * fp = fopen ("xy.dat", "w");

  node_t * p = n;

  for (int i = 0; i < 10; i++)
    {
      p = p->getPrev ();
      i++;
      if (p == n)
        break;
    }

  for (int i = 0; i < 20; i++)
    {
      int j = p->getRank ();
      fprintf (fp, "%8d %12.2f %12.2f\n", j, rad2deg * xy[2*j+0], 
               rad2deg * xy[2*j+1]);
      p = p->getNext ();
    }


  fclose (fp);
}


{
  int j = 297;
  int j1 = 967;
  node_t * n1;


  for (n1 = nodelist; ; )
    {
      n1 = n1->getNext ();

      if (n1->getRank () == j1)
        break;

      if (n1 == nodelist)
        return;
    }

  

  const glm::vec3 & p = n1->getXYZ (xyz);
  if (n->inTriangle (p, xyz))
    std::cout << j1 << " in " << j << std::endl;
}

}

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
  bool contains (float x)
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
    bool cmid = contains ((inter.xmin + inter.xmax) / 2.0f);

    if (cmin && cmax && cmid)
      return;

    if (cmin && cmax)
      {
        full = true;
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
        

void glgrib_test::setup ()
{


if(0)
{
  glm::vec2 lonlat1 (deg2rad *   0.0f, deg2rad * -10.0f);
  glm::vec2 lonlat2 (deg2rad * 180.0f, deg2rad * -10.0f);

  float latmin, latmax;

  getLonLatRange (lonlat1, lonlat2, &latmin, &latmax);

  std::cout << " latmin, latmax = " << rad2deg * latmin << ", " << rad2deg * latmax << std::endl;

  exit (0);

}

  int numberOfPoints;
  glgrib_options_lines opts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<float> xy;
  std::vector<unsigned int> indl;
  
  opts.path = "coastlines/shp/GSHHS_i_L1.shp";
  opts.path = "coastlines/shp/GSHHS_h_L1.shp";
  opts.path = "coastlines/shp/GSHHS_f_L1.shp";
  opts.path = "coastlines/shp/GSHHS_c_L1.shp";

  opts.selector = "rowid == 1";
  glgrib_shapelib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &indl, opts.selector);

  std::vector<glm::vec3> xyz;
  std::vector<float> ang;

  int numberOfPoints1 = numberOfPoints-1;

  ang.resize (numberOfPoints1);
  xyz.resize (numberOfPoints1);

#pragma omp parallel for
  for (int i = 0; i < numberOfPoints1; i++)
    {
      float lon = lonlat[2*i+0], lat = lonlat[2*i+1];
      float coslon = cos (lon), sinlon = sin (lon);
      float coslat = cos (lat), sinlat = sin (lat);
      xyz[i] = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
    }


  xy = lonlat;

  std::vector<node_t> nodevec;

  nodevec.reserve (numberOfPoints1);

  for (int j = 0; j < numberOfPoints1; j++)
    nodevec.push_back (node_t (j));

#pragma omp parallel for
  for (int j = 0; j < numberOfPoints1; j++)
    {
      int i = j == 0 ? numberOfPoints1-1 : j-1;
      int k = j == numberOfPoints1-1 ? 0 : j+1;
      nodevec[j].setPrevNext (&nodevec[i], &nodevec[k]);
      nodevec[j].getAngle (xyz);
    }


  inter_t minmax;

  int i = 0;
  for (node_t * n1 = &nodevec[0]; ; )
    {
      node_t * n2 = n1->getNext ();

      float x1 = n1->getX (xy);
      float x2 = n2->getX (xy);

      xint (x1, x2);

      if (i == 0)
        {
          minmax.xmin = x1;
          minmax.xmax = x2;
        }
      else
        {
          minmax.expand (inter_t (x1, x2));
        }

      i++;
      if ((n1 = n2) == &nodevec[0])
        break;
    }

  std::cout << " xmin, xmax = " << rad2deg * minmax.xmin << ", " << rad2deg * minmax.xmax << std::endl;

//exit (0);


  xy2node_t ll2n;

  ll2n.init (nodevec, xy, xyz);


  std::vector<node_t*> ain;

  ain.reserve (numberOfPoints1);

  for (int j = 0; j < numberOfPoints1; j++)
    if (nodevec[j].getAngle (xyz) < 0.0f)
      ain.push_back (&nodevec[j]);

  node_t * nodelist = &nodevec[0];

  std::vector<unsigned int> ind;




FILE * fp = fopen ("LOG.txt", "w");

for (int i = 0; i < 22; i++)
{

  if (i == 15) {
  dump (nodelist, ain, ll2n, xyz, xy, std::to_string (i));
  pr (xy, xyz, ll2n, 297, nodelist);
  }

  fprintf (fp, " %8d %8d %8ld\n", i, nodelist->count (), ain.size ());
  earCut (&nodelist, xyz, xy, ll2n, &ind, &ain);


  if (i == 15)
    break;

  fprintf (fp, "----LIST---- %d\n", i);
  for (node_t * n = nodelist; ; )
    {
      fprintf (fp, " %8d", n->getRank ());
      n = n->getNext ();
      if (n == nodelist)
        break;
    }
  fprintf (fp, "\n");

  ll2n.update (xyz);

}
  fprintf (fp, " %8s %8d %8ld\n", "", nodelist->count (), ain.size ());

fclose (fp);




  numberOfTriangles = ind.size () / 3;

  vertexbuffer = new_glgrib_opengl_buffer_ptr (xyz.size () * sizeof (xyz[0]), xyz.data ());
  elementbuffer = new_glgrib_opengl_buffer_ptr (ind.size () * sizeof (ind[0]), ind.data ());

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  vertexbuffer->bind (GL_ARRAY_BUFFER);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
  elementbuffer->bind (GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray (0); 

  setReady ();
}

