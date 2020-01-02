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

  bool inTriangle (const glm::vec3 & p, const std::vector<glm::vec3> & xyz) const
  {
    const glm::vec3 t[3] = {xyz[prev->rank], xyz[rank], xyz[next->rank]};
  
    float S;
  
    for (int i = 0; i < 3; i++)
      {
        int j = (i + 1) % 3;
        const glm::vec3 u = glm::cross (t[i], t[j] - t[i]);
        float s = glm::dot (p - t[i], u);
        if (i == 0)
          S = s;
        else if (s * S < 0.0f)
          return false;
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


class lonlat2node_t
{
public:
  void init (const std::vector<node_t> & nodevec,
             const std::vector<float> & lonlat, 
             const std::vector<glm::vec3> & xyz)
  {

    dlon = twopi / nlon;
    dlat = pi / nlat;

    off.resize (nlon * nlat);
    len.resize (nlon * nlat);

    for (int k = 0; k < len.size (); k++)
      len[k] = 0;

    for (int i = 0; i < nodevec.size (); i++)
      {
        if (nodevec[i].getAngle (xyz) < 0.0f)
          {
            int ind = lonlat2ind (lonlat[2*i+0], lonlat[2*i+1]);
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
            int ind = lonlat2ind (lonlat[2*i+0], lonlat[2*i+1]);
            nodes[cur[ind]] = &nodevec[i];
            cur[ind]++;
          }
      }
  }

  void update (const std::vector<glm::vec3> & xyz)
  {
#pragma omp parallel for
    for (int i = 0; i < len.size (); i++)
      for (int j = off[i]; j < off[i] + len[i]; j++)
        if (nodes[j]->getAngle (xyz) < 0.0f) 
          {
            int k = off[i]+len[i]-1;
            std::swap (nodes[j], nodes[k]);
            len[i]--;
          }
  }

  int lonlat2ind (float lon, float lat) const
  {
    while (lon <     0) lon += twopi;
    while (lon > twopi) lon -= twopi;
    int ilon = nlon * lon / twopi;
    int ilat = nlat * (lat + halfpi) / pi;
    int ind = ilon + ilat * nlon;
    if (ind >= off.size ())
      abort ();
    return ind;
  }

  int lonlat2ind (int ilon, int ilat) const
  {
    while (ilon <    0) ilon += nlon;
    while (ilon > nlon) ilon -= nlon;
    int ind = ilon + ilat * nlon;
    if (ind >= off.size ())
      abort ();
    return ind;
  }

  class iterator 
  {
  public:
    iterator (float _latmin, float _latmax, float _lonmin, float _lonmax, const lonlat2node_t * _ll2n) 
    : latmin (_latmin), latmax (_latmax), lonmin (_lonmin), lonmax (_lonmax), ll2n (_ll2n)
    {

static 
bool dbg = true;

      ilatmin = floor ((latmin + halfpi) / ll2n->dlat), ilatmax = floor ((latmax + halfpi) / ll2n->dlat);
      ilonmin = floor ( lonmin           / ll2n->dlon), ilonmax = floor ( lonmax           / ll2n->dlon);

      nlat = ilatmax - ilatmin + 1;
      nlon = ilonmax - ilonmin + 1;

if(dbg){
      std::cout << " ilatmin = " << ilatmin << " ilatmax = " << ilatmax << std::endl;
      std::cout << " ilonmin = " << ilonmin << " ilonmax = " << ilonmax << std::endl;
}

      lonmin = ilonmin * ll2n->dlon         ; lonmax = ilonmax * ll2n->dlon         ;
      latmin = ilatmin * ll2n->dlat - halfpi; latmax = ilatmax * ll2n->dlat - halfpi;

if(dbg){
      std::cout << "  latmin = " << rad2deg * latmin << "  latmax = " << rad2deg * latmax << std::endl;
      std::cout << "  lonmin = " << rad2deg * lonmin << "  lonmax = " << rad2deg * lonmax << std::endl;
}

dbg = false;

      this->seek (0);
    }
    void seek (int whence)
    {
      if (whence == 0)
        {
          ind = ll2n->lonlat2ind (ilonmin, ilatmin);
          cur = 0;
          ilat = 0; ilon = 0;
          if (ll2n->len[ind] == 0)
            operator++ ();
        }
      else
        {
          cur = 0; ilat = nlat; ilon = 0;
        }
    }

    iterator & operator++ ()
    {
       cur++;
       while (cur >= ll2n->len[ind])
         {
           cur = 0;
           ilon++;
           if (ilon >= nlon)
             {
               ilon = 0;
               ilat++;
               if (ilat == nlat)
                 break;
             }
           ind = ll2n->lonlat2ind (ilonmin + ilon, 
                                   ilatmin + ilat);
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
      return (ll2n == rhs.ll2n) && (ilon == rhs.ilon) && 
             (ilat == rhs.ilat) && (cur == rhs.cur);
    }

    std::string asString () const
    {
      int ilon1 = ind % ll2n->nlon;
      int ilat1 = ind / ll2n->nlon;
      float lon1 = ilon1 * ll2n->dlon, lon2 = lon1 + ll2n->dlon;
      float lat1 = ilat1 * ll2n->dlat - halfpi, lat2 = lat1 + ll2n->dlat;
      
      return std::string ("iterator{") +
             "ilon=" + std::to_string (ilon) + "/" + std::to_string (nlon)           + ", " +
             "ilat=" + std::to_string (ilat) + "/" + std::to_string (nlat)           + ", " +
             "cur =" + std::to_string (cur)  + "/" + std::to_string (ll2n->len[ind]) + ", " +
             "ind =" + std::to_string (ind)  +                                         ", " +
             "off =" + std::to_string (ll2n->off[ind])     + ", " +
             "len =" + std::to_string (ll2n->len[ind])     + ", " +
             "    =" + std::to_string (ll2n->off[ind]+cur) +
             "[" + std::to_string (rad2deg * lon1) + ".." + std::to_string (rad2deg * lon2) + "]" +
             "[" + std::to_string (rad2deg * lat1) + ".." + std::to_string (rad2deg * lat2) + "]" +
             "}";
    }

  private:
    int ind; // Current index in ll2n
    int cur; // Current index in nodes
    int ilat, ilon;
    int nlat, nlon;
    float  latmin,  latmax,  lonmin,  lonmax;
    int   ilatmin, ilatmax, ilonmin, ilonmax;
    const lonlat2node_t * ll2n = NULL;
  };

  iterator begin (const node_t & n, const std::vector<float> & lonlat) const
  {
    float lat0 = n.            getLat (lonlat);
    float lat1 = n.getNext ()->getLat (lonlat);
    float lat2 = n.getPrev ()->getLat (lonlat);

    float latmin = std::min (lat0, std::min (lat1, lat2));
    float latmax = std::max (lat0, std::max (lat1, lat2));

    float lon0 = n.            getLon (lonlat);
    float lon1 = n.getNext ()->getLon (lonlat);
    float lon2 = n.getPrev ()->getLon (lonlat);


    while (lon1 - lon0 > +twopi) lon1 -= twopi;
    while (lon1 - lon0 < -twopi) lon1 += twopi;
    while (lon2 - lon0 > +twopi) lon2 -= twopi;
    while (lon2 - lon0 < -twopi) lon2 += twopi;

    float lonmin = std::min (lon0, std::min (lon1, lon2));
    float lonmax = std::max (lon0, std::max (lon1, lon2));

    return iterator (latmin, latmax, lonmin, lonmax, this);
  }

  iterator end (const node_t & n, const std::vector<float> & lonlat) const
  {
    iterator it = begin (n, lonlat);
    it.seek (1);
    return it;
  }


private:
  friend class iterator;
  int nlon = 360, nlat = 180;
  std::vector<int> off, len;
  std::vector<const node_t*> nodes;
  float dlon, dlat;
  
};


std::ostream & operator<< (std::ostream& os, const lonlat2node_t::iterator & it)
{
  os << it.asString ();
  return os;
}



static 
void earCut (node_t ** nodelist,  
             const std::vector<glm::vec3> & xyz,
             std::vector<unsigned int> * ind,
             std::vector<node_t*> * ain)
{

  node_t * last = (*nodelist)->getPrev ();

  for (node_t * n = *nodelist; ; )
    {
      float ang = n->getAngle (xyz);

      if ((0.0f < ang) && (ang < M_PI))
        {
  
          volatile 
          int intri = 0;

#pragma omp parallel
          {
          int nt = omp_get_num_threads ();
          int it = omp_get_thread_num ();

          int i1 = (ain->size () * (it + 0)) / nt;
          int i2 = (ain->size () * (it + 1)) / nt;

          for (int i = i1; i < i2; i++)
            {
              node_t * n1 = (*ain)[i];
              if ((n1 != n) && n1->linked () && (n1 != n->getNext ()) && 
                  (n1 != n->getPrev ()) && (n1->getAngle (xyz) <= 0.0f))
                {
                  const glm::vec3 & p = n1->getXYZ (xyz);
                  if (n->inTriangle (p, xyz))
                    {
#pragma omp atomic
                      intri++;
                    }
                }
              if (intri)
                break;
            }
          }

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


void glgrib_test::setup ()
{

  int numberOfPoints;
  glgrib_options_lines opts;
  unsigned int numberOfLines; 
  std::vector<float> lonlat;
  std::vector<unsigned int> indl;
  
  opts.path = "coastlines/shp/GSHHS_i_L1.shp";
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


  lonlat2node_t ll2n;

  ll2n.init (nodevec, lonlat, xyz);



  for (int jj = 572; jj < 573; jj++) {

  
  std::cout << jj << " -> " << nodevec[jj] << std::endl;

  lonlat2node_t::iterator it = ll2n.begin (nodevec[jj], lonlat);
  lonlat2node_t::iterator it1 = ll2n.end (nodevec[jj], lonlat);
  for ( ; it != it1; ++it)
    {
      const node_t * n = *it;
      int rank = n->getRank ();
      std::cout << it << " " << *n << " ";
      printf (" %12.2f %12.2f %12.2f", 
              rad2deg * lonlat[2*rank+0], 
              rad2deg * lonlat[2*rank+1], 
              rad2deg * n->getAngle (xyz));
      std::cout << std::endl;
    }

  }



  std::vector<node_t*> ain;

  ain.reserve (numberOfPoints1);

  for (int j = 0; j < numberOfPoints1; j++)
    if (nodevec[j].getAngle (xyz) < 0.0f)
      ain.push_back (&nodevec[j]);

  std::vector<unsigned int> ind;


{
  FILE * fp = fopen ("ain.dat", "w");
  for (int j = 0; j < nodevec.size (); j++)
    {
      std::vector<int> ii;

      const node_t * n = &nodevec[j];
      for (int i = 0; i < ain.size (); i++)
        {
          const node_t * n1 = ain[i];
          const glm::vec3 & p = n1->getXYZ (xyz);
          if (n->inTriangle (p, xyz))
            ii.push_back (n1->getRank ());
        }
      if (ii.size ())
        {
          int rank = n->getRank ();
          sort (ii.begin (), ii.end ()); 
          fprintf (fp, "%8d | %12.2f %12.2f %12.2f :", rank,
                   rad2deg * n->getAngle (xyz), lonlat[2*rank+0] * rad2deg, lonlat[2*rank+1] * rad2deg);
          for (int i = 0; i < ii.size (); i++)
            fprintf (fp, " %8d", ii[i]);
          fprintf (fp, "\n");
        } 
    }

  fclose (fp);
}
{
  FILE * fp = fopen ("ll2.dat", "w");
  for (int j = 0; j < nodevec.size (); j++)
    {
      std::vector<int> ii;

      const node_t * n = &nodevec[j];
      lonlat2node_t::iterator it = ll2n.begin (nodevec[j], lonlat);
      lonlat2node_t::iterator it1 = ll2n.end (nodevec[j], lonlat);
      for ( ; it != it1; ++it)
        {
          const node_t * n1 = *it;
          const glm::vec3 & p = n1->getXYZ (xyz);
          if (n->inTriangle (p, xyz))
            ii.push_back (n1->getRank ());
        }
      if (ii.size ())
        {
          int rank = n->getRank ();
          sort (ii.begin (), ii.end ()); 
          fprintf (fp, "%8d | %12.2f %12.2f %12.2f :", rank,
                   rad2deg * n->getAngle (xyz), lonlat[2*rank+0] * rad2deg, lonlat[2*rank+1] * rad2deg);
          for (int i = 0; i < ii.size (); i++)
            fprintf (fp, " %8d", ii[i]);
          fprintf (fp, "\n");
        } 
    }
  fclose (fp);
}


  node_t * nodelist = &nodevec[0];

for (int i = 0; i < 22; i++)
{
//printf (" %8d %8d %8ld\n", i, nodelist->count (), ain.size ());
  earCut (&nodelist, xyz, &ind, &ain);
}
//printf (" %8s %8d %8ld\n", "", nodelist->count (), ain.size ());


{
  FILE * fp = fopen ("lonlat.dat", "w");

  for (int i = 550; i < 590; i++)
    {
      int j = i < 0 ? i + nodevec.size () : i;
      fprintf (fp, "%8d %12.2f %12.2f\n", j, rad2deg * lonlat[2*j+0], rad2deg * lonlat[2*j+1]);
    }


  fclose (fp);
}




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

