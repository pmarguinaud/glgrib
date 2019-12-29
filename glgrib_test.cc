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


static
void earCut (const std::vector<glm::vec3> xyz, 
             const std::vector<float> ang, 
             std::vector<unsigned int> * ind)
{
  int numberOfPoints1 = ang.size ();

  for (int j = 0; j < numberOfPoints1; j++)
    {

      if ((0.0f < ang[j]) && (ang[j] < M_PI))
        {
          int i = j == 0 ? numberOfPoints1-1 : j-1;
          int k = j == numberOfPoints1-1 ? 0 : j+1;

bool intri = false;
for (int j1 = 0; j1 < numberOfPoints1; j1++)
  {
    if ((j1 == i) || (j1 == j) || (j1 == k))
      continue;
    if (ang[j1] <= 0.0)
      intri = intri || inTriangle (xyz[j1], xyz[i], xyz[j], xyz[k]);
  }

if (intri)
  std::cout << j << std::endl;
          if (! intri)
            {
              ind->push_back (i);
              ind->push_back (j);
              ind->push_back (k);
            }
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



  for (int j = 0; j < numberOfPoints1; j++)
    {
      int i = j == 0 ? numberOfPoints1-1 : j-1;
      int k = j == numberOfPoints1-1 ? 0 : j+1;
      glm::vec3 ji = glm::cross (xyz[j], xyz[i] - xyz[j]);
      glm::vec3 jk = glm::cross (xyz[j], xyz[k] - xyz[j]);
      float X = glm::dot (ji, jk);
      float Y = glm::dot (xyz[j], glm::cross (ji, jk));
      ang[j] = atan2 (Y, X);


#ifdef UNDEF
bool dbg = j < 10;

      if (dbg)
        {
          printf ("%3d\n", j);
          printf ("xyz = %8.2f, %8.2f, %8.2f\n", xyz[j].x, xyz[j].y, xyz[j].z);
          printf ("ji  = %8.2f, %8.2f, %8.2f\n", ji    .x, ji    .y, ji    .z);
          printf ("jk  = %8.2f, %8.2f, %8.2f\n", jk    .x, jk    .y, jk    .z);
          printf ("ang = %8.2f\n", rad2deg * ang[j]);

          printf ("\n");
        }
#endif


    }

  
#ifdef UNDEF
  FILE * fp = fopen ("lonlat.dat", "w");

  for (int j = -10; j < +10; j++)
    {
      int i = j < 0 ? j + numberOfPoints1 : j;
      fprintf (fp, " %6d %8.2f %8.2f %8.2f\n", i, rad2deg * lonlat[2*i+0], rad2deg * lonlat[2*i+1], rad2deg * ang[i]);
    }

  fclose (fp);
#endif



  std::vector<unsigned int> ind;

  earCut (xyz, ang, &ind);

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

