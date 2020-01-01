#include "glgrib_test.h"
#include "glgrib_opengl.h"
#include "glgrib_shapelib.h"
#include "glgrib_options.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>


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


class ind_t
{
public:
  ind_t (int _i, int _j) : i (_i), j (_j) { }
  int i, j;
};

template <typename T>
class vecvec_t
{
public:


  class iterator
  {
  public:
    iterator (vecvec_t * _vv, int i = 0, int j = 0) : vv (_vv), ind (i, j) { }
    iterator operator++ (int)
    {
       iterator it1 (vv, ind.i, ind.j);
       ind.j++;
       if (ind.j == vv->d[ind.i]->size ())
         {
           ind.j = 0;
           ind.i++;
         }
       return it1;
    }
    T * operator->() 
    {
      return &(*vv)[ind];
    }
    bool isEqual (const iterator & rhs) const
    {
      return (ind.i == rhs.ind.i) && (ind.j == rhs.ind.j);
    }
    bool operator!= (const iterator & rhs) const
    {
      return ! isEqual (rhs);
    }
  private:
    vecvec_t * vv;
    ind_t ind;
  };

  iterator begin () 
  {
    return iterator (this);
  }

  iterator end () 
  {
    return iterator (this, d.size (), 0);
  }

  T & operator[] (ind_t ii) const
  {
    return (*(d[ii.i]))[ii.j];
  }

  void push (std::vector<T> * v)
  {
    d.push_back (v);
  }

  ~vecvec_t ()
  {
    for (int i = 0; i < d.size (); i++)
      delete d[i];
  }

  size_t size () const
  {
    size_t s = 0;
    for (int i = 0; i < d.size (); i++)
      s += d[i]->size ();
    return s;
  }

  size_t size1 () const
  {
    return d.size ();
  }

private:
  friend class iterator;

  std::vector<std::vector<T>*> d;
};

const float rad2deg = 180.0f / M_PI;

class node_t
{
public:

  node_t (int _rank, const glm::vec3 & _xyz) : rank (_rank), xyz (_xyz) {}

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

  float getAngle () const 
  {
    if (dirty)
      {
        glm::vec3 ji = glm::cross (xyz, prev->xyz - xyz);
        glm::vec3 jk = glm::cross (xyz, next->xyz - xyz);
        float X = glm::dot (ji, jk);
        float Y = glm::dot (xyz, glm::cross (ji, jk));
        ang = atan2 (Y, X);
        dirty = false;
      }
    return ang;
  }

  float getDist () const 
  {
    const glm::vec3 & xyzn = next->getXYZ ();
    const glm::vec3 & xyzp = prev->getXYZ ();
    return acos (std::max (-1.0f, 
                 std::min (+1.0f, 
                           glm::dot (xyzn, xyzp))));
  }

  const glm::vec3 & getXYZ () const
  {
    return xyz;
  }

  bool inTriangle (const glm::vec3 & p) const
  {
    const glm::vec3 t[3] = {prev->xyz, xyz, next->xyz};
  
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

  void cut ()
  {
    node_t * n = next, * p = prev;

    n->setPrev (prev);
    p->setNext (next);
    next = prev = NULL; 
    
  }

  void replace (node_t * n)
  {
    n->next->prev = this;
    n->prev->next = this;
    next = n->next;
    prev = n->prev;
    n->next = NULL;
    n->prev = NULL;
  }

  bool isLinked () const
  {
    return (prev != NULL) && (next != NULL);
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

  void setRank (int _rank)
  {
    rank = _rank;
  }


private:
  int rank = -1;
  node_t * prev = NULL, * next = NULL;
  glm::vec3 xyz;

  mutable float ang = 0.0f;
  mutable bool dirty = true;
  
};


static 
void earCut (vecvec_t<node_t> & vv,
             std::vector<ind_t> & polygon,
             std::vector<ind_t> & ainside,
             std::vector<unsigned int> * ind)
{
  std::vector<ind_t> polygon1;
  std::vector<node_t*> listcut, listmov;
  std::vector<node_t> * listnew = NULL;


  polygon1.reserve (polygon.size ());

  for (int i = 0; i < polygon.size ()-1; i++)
    {

      node_t * n = &vv[polygon[i]];

      float ang = n->getAngle ();

      if ((0.0f < ang) && (ang < M_PI))
        {
  
          bool intri = false;

          for (std::vector<ind_t>::const_iterator it = ainside.begin ();
               it != ainside.end (); it++)
            {
              const node_t * n1 = &vv[*it];
              if ((n1 != n) && (n1 != n->getNext ()) && (n1 != n->getPrev ()))
                {
                  const glm::vec3 & p = n1->getXYZ ();
                  intri = n->inTriangle (p);
                  if (intri)
                    break;
                }
            }

          if (! intri)
            {
              
              const float dmax = M_PI / 180.0f;

              if (n->getDist () > dmax)
                {
                  listmov.push_back (n);
                  if (listnew == NULL)
                    listnew = new std::vector<node_t>();
                  glm::vec3 xyz = glm::normalize (n->getPrev ()->getXYZ () 
                                                + n->getNext ()->getXYZ ());
                  node_t n1 (-1, xyz);
                  n1.replace (n);
                  listnew->push_back (n1);
                }
              else
                {
                  listcut.push_back (n);
                }

              i++;

            }

        }

      if (i < polygon.size ())
        polygon1.push_back (polygon[i]);
    }

  int ind_base = ind->size ();
  ind->resize (ind_base + 3 * listcut.size ());
  for (int i = 0; i < listcut.size (); i++)
    {
      node_t * n = listcut[i];
      (*ind)[ind_base+3*i+0] = n->getPrev ()->getRank ();
      (*ind)[ind_base+3*i+1] = n            ->getRank ();
      (*ind)[ind_base+3*i+2] = n->getNext ()->getRank ();
      n->cut ();
    }


  if (listnew)
    {
      int round = vv.size1 (), rank_base = vv.size ();

      vv.push (listnew);

      int ind_base = ind->size ();
      ind->resize (ind_base + 6 * listnew->size ());
     
      for (int i = 0; i < listnew->size (); i++)
        {
          node_t * n = listmov[i];
          node_t * n1 = &(*listnew)[i];

          n1->setRank (rank_base + i);

          (*ind)[ind_base+6*i+0] = n1->getPrev ()->getRank ();
          (*ind)[ind_base+6*i+1] = n1            ->getRank ();
          (*ind)[ind_base+6*i+2] = n             ->getRank ();
 
          (*ind)[ind_base+6*i+3] = n1->getNext ()->getRank ();
          (*ind)[ind_base+6*i+4] = n1            ->getRank ();
          (*ind)[ind_base+6*i+5] = n             ->getRank ();

        }

    }

  polygon = polygon1;


  for (int i = 0; i < ainside.size (); i++)
    {
      const node_t * n = &vv[ainside[i]];
      if (! n->isLinked ())
        {
          std::swap (ainside[i], ainside.back ());
          ainside.pop_back ();
        }
      else if (n->getAngle () >= 0.0f)
        {
          std::swap (ainside[i], ainside.back ());
          ainside.pop_back ();
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
  
  opts.path = "coastlines/shp/GSHHS_c_L1.shp";
  opts.selector = "rowid == 1";
  glgrib_shapelib::read (opts, &numberOfPoints, &numberOfLines, &lonlat, &indl, opts.selector);

  int numberOfPoints1 = numberOfPoints-1;

  std::vector<node_t> * nodevec = new std::vector<node_t>();
  std::vector<ind_t> polygon;
  std::vector<ind_t> ainside;

  nodevec->reserve (numberOfPoints1);

  for (int j = 0; j < numberOfPoints1; j++)
    {
      float lon = lonlat[2*j+0], lat = lonlat[2*j+1];
      float coslon = cos (lon), sinlon = sin (lon);
      float coslat = cos (lat), sinlat = sin (lat);
      nodevec->push_back (node_t (j, glm::vec3 (coslon * coslat, sinlon * coslat, sinlat)));
    }


  for (int j = 0; j < numberOfPoints1; j++)
    {
      int i = j == 0 ? numberOfPoints1-1 : j-1;
      int k = j == numberOfPoints1-1 ? 0 : j+1;
      (*nodevec)[j].setPrevNext (&(*nodevec)[i], &(*nodevec)[k]);
      polygon.push_back (ind_t (0, j));
      if ((*nodevec)[j].getAngle () <= 0.0f)
        ainside.push_back (ind_t (0, j));
    }

  vecvec_t<node_t> vv;

  vv.push (nodevec);

  std::vector<unsigned int> ind;

  


  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;

if(0){
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
  earCut (vv, polygon, ainside, &ind);
  std::cout << polygon.size () << ", " << ainside.size () << std::endl;
}

  std::vector<glm::vec3> xyz;

  xyz.reserve (vv.size ());

  for (vecvec_t<node_t>::iterator it = vv.begin (); it != vv.end (); it++)
    xyz.push_back (it->getXYZ ());


  for (int i = 0; i < xyz.size (); i++)
    if (glm::length (xyz[i]) < 0.1f)
    printf (" !!!!! %8d | %8.2f | %8.2f | %8.2f\n", i, xyz[i].x, xyz[i].y, xyz[i].z);

  for (int i = 0; i < ind.size (); i++)
    if (ind[i] >= xyz.size ())
      printf (" !!!!! ind %8d %8u\n", i, ind[i]);


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

