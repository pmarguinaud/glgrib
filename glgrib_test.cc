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

const float rad2deg = 180.0f / M_PI;


static 
bool inTriangle (const glm::vec3 & p, const glm::vec3 & a1, const glm::vec3 & a2, const glm::vec3 & a3)
{
  const glm::vec3 t[3] = {a1, a2, a3};

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

class node_t
{
public:

  node_t (int _rank) : rank (_rank) {}

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


static 
void earCut (node_t ** nodelist,  
             const std::vector<glm::vec3> & xyz,
             std::vector<unsigned int> * ind)
{

  for (node_t * n = *nodelist; ; )
    {
      float ang = n->getAngle (xyz);

      if ((0.0f < ang) && (ang < M_PI))
        {
  
          bool intri = false;
          for (const node_t * n1 = *nodelist; ;)
            {
              if ((n1->getAngle (xyz) <= 0.0f) && (n1 != n) && 
                  (n1 != n->getNext ()) && (n1 != n->getPrev ()))
                {
                  const glm::vec3 & p = n1->getXYZ (xyz);
                  intri = n->inTriangle (p, xyz);
                  if (intri)
                    break;
                }

              n1 = n1->getNext ();
              if (n1 == *nodelist)
                break;
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
      if (n == *nodelist)
        break;
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

  std::vector<glm::vec3> xyz;
  std::vector<float> ang;

  int numberOfPoints1 = numberOfPoints-1;

  ang.resize (numberOfPoints1);
  xyz.resize (numberOfPoints1);

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

  for (int j = 0; j < numberOfPoints1; j++)
    {
      int i = j == 0 ? numberOfPoints1-1 : j-1;
      int k = j == numberOfPoints1-1 ? 0 : j+1;
      nodevec[j].setPrevNext (&nodevec[i], &nodevec[k]);
    }

  std::vector<unsigned int> ind;

  node_t * nodelist = &nodevec[0];

  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;
  earCut (&nodelist, xyz, &ind);
  std::cout << nodelist->count () << std::endl;

  int k = 0;

  FILE * fp = fopen ("nodelist.dat", "w");
  for (node_t * n = nodelist; ; )
    {
      int r = n->getRank ();

      fprintf (fp, " %8d %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f \n", r, xyz[r].x, 
               xyz[r].y, xyz[r].z, rad2deg * lonlat[2*r+0], rad2deg * lonlat[2*r+1],
               rad2deg * n->getAngle (xyz));

      n = n->getNext ();
      if (n == nodelist)
        break;
      k++;

      if (k > 10)
        break;
    }
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

