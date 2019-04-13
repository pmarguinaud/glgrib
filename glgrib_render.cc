#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef USE_GLE

#define GL_GLEXT_PROTOTYPES 1
#include <GL/glut.h>
#include <GL/glext.h>

#else

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#endif

#include "glgrib_render.h"
#include "glgrib_load.h"

using namespace glm;

#include "shader.h"

void scene_t::display () const
{
  float xc = view->rc * glm::cos (glm::radians (view->lonc)) * glm::cos (glm::radians (view->latc)), 
        yc = view->rc * glm::sin (glm::radians (view->lonc)) * glm::cos (glm::radians (view->latc)),
        zc = view->rc *                                        glm::sin (glm::radians (view->latc));

  glm::mat4 Projection = glm::perspective (glm::radians (view->fov), 1.0f / 1.0f, 0.1f, 100.0f);
  glm::mat4 View       = glm::lookAt (glm::vec3 (xc,yc,zc), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 Model      = glm::mat4 (1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glUseProgram (prog->programID);
  glUniformMatrix4fv (view->MatrixID, 1, GL_FALSE, &MVP[0][0]);
  
  for (std::list<obj_t*>::const_iterator it = objlist.begin (); 
       it != objlist.end (); it++)
    (*it)->render ();

}

void polygon_t::def_from_xyz_col_ind (const float * xyz, const unsigned char * col, const unsigned int * ind)
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 

  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL);

  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 2 * nl * sizeof (unsigned int), ind, GL_STATIC_DRAW);
}

void polygon_t::render () const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_LINES, 2 * nl, GL_UNSIGNED_INT, NULL);
}

polygon_t::~polygon_t ()
{
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteBuffers (1, &colorbuffer);
  glDeleteVertexArrays (1, &VertexArrayID);
}

typedef struct head_t
{
  int id;                       /* Unique polygon id number, starting at 0 */
  int n;                        /* Number of points in this polygon */
  int level;                    /* 1 land, 2 lake, 3 island_in_lake, 4 pond_in_island_in_lake */
  int west, east, south, north; /* min/max extent in micro-degrees */
  int area;                     /* Area of polygon in 1/10 km^2 */
  short int greenwich;          /* Greenwich is 1 if Greenwich is crossed */
  short int source;             /* 0 = CIA WDBII, 1 = WVS */
} head_t;

typedef struct point_t
{
  int x; /* longitude of a point in micro-degrees */
  int y; /* latitude of a point in micro-degrees */
} point_t;

void grid_t::init ()
{
  ncol = use_alpha ? 4 : 3;

  float * xyz = NULL;
  unsigned char * col = NULL;
  unsigned int * ind = NULL;

  head_t h;
  point_t * pl = NULL;
  int ip = 0, il = 0;
  np = 0;
  nl = 0;

  float r = 1.005;
  const int nlatv = 200, nlonv = 20;
  const int nlath = 10, nlonh = 400;

  for (int pass = 0; pass < 2; pass++)
    {

      for (int jlon = 0; jlon < nlonv; jlon++)
        {
          float zlon = 2. * M_PI * (float)jlon / (float)nlonv;
          float coslon = cos (zlon);
          float sinlon = sin (zlon);
          for (int jlat = 0; jlat < nlatv+1; jlat++)
            {   
              float zlat = M_PI / 2. - M_PI * (float)jlat / (float)nlatv;
              float coslat = cos (zlat);
              float sinlat = sin (zlat);
              if (pass == 1)
                {
                  xyz[ip*3+0] = r * coslon * coslat;
                  xyz[ip*3+1] = r * sinlon * coslat;
                  xyz[ip*3+2] = r *          sinlat;
                  if (jlat < nlatv)
                    {
                      ind[il*2+0] = ip;
                      ind[il*2+1] = ip + 1;
                      il++;
                    }
                  ip++;
                }
              else
                {
                  np++;
                  if (jlat < nlatv)
                    nl++;
                }
            }
        }
      for (int jlat = 1; jlat < nlath; jlat++)   
        {
          float zlat = M_PI / 2. - M_PI * (float)jlat / (float)nlath;
          float coslat = cos (zlat);
          float sinlat = sin (zlat);
          int nloen = (int)(nlonh * coslat);
          int ip0 = ip;
          for (int jlon = 0; jlon < nloen; jlon++)
            {
              float zlon = 2. * M_PI * (float)jlon / (float)nloen;
              float coslon = cos (zlon);
              float sinlon = sin (zlon);

              if (pass == 1) 
                {
                  xyz[ip*3+0] = r * coslon * coslat;
                  xyz[ip*3+1] = r * sinlon * coslat;
                  xyz[ip*3+2] = r *          sinlat;
                  ind[il*2+0] = ip;
                  if (jlon < nloen-1)
                    ind[il*2+1] = ip + 1;
                  else
                    ind[il*2+1] = ip0;
                  ip++;
                  il++;
                }
              else
                {
                  np++;
                  nl++;
                }

            }
        }

      if (pass == 0)
        {
          xyz = (float *)malloc (3 * np * sizeof (float));
          col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
          ind = (unsigned int *)malloc (nl * 2 * sizeof (unsigned int));
	}

    }

  free (pl);

  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 255;

  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}

void coastlines_t::init (const char * file)
{
  ncol = use_alpha ? 4 : 3;

  float * xyz = NULL;
  unsigned char * col = NULL;
  unsigned int * ind = NULL;

  head_t h;
  point_t * pl = NULL;
  FILE * fp = NULL;
  int ip = 0, il = 0;
  np = 0;
  nl = 0;

  float r = 1.005;
  const float millideg2rad = M_PI / (1000000. * 180.);

  for (int pass = 0; pass < 2; pass++)
    {
      fp = fopen (file, "r");
     
      while (1) 
        {   
          fread (&h, sizeof (h), 1, fp);
          pl = (point_t *)realloc (pl, h.n * sizeof (point_t));
          fread (pl, sizeof (point_t), h.n, fp);
          if (h.level == 1)
	    {
	      if (pass == 0)
                {
                  np += h.n;
	          nl += h.n;
	        }
	      else
                {
                  int ip0 = ip;
                  for (int i = 0; i < h.n; i++)
	            {
                      float coslon = cos (millideg2rad * pl[i].x);
                      float sinlon = sin (millideg2rad * pl[i].x);
                      float coslat = cos (millideg2rad * pl[i].y);
                      float sinlat = sin (millideg2rad * pl[i].y);
                      xyz[ip*3+0] = r * coslon * coslat;
                      xyz[ip*3+1] = r * sinlon * coslat;
                      xyz[ip*3+2] = r *          sinlat;
	              ind[il*2+0] = ip;
	              if (i == h.n - 1)
                        ind[il*2+1] = ip0;
	              else
                        ind[il*2+1] = ip + 1;
	              ip++;
	              il++;
	            }
	        }
            }
          if (feof (fp))
            break;
        }   

      if (pass == 0)
        {
          xyz = (float *)malloc (3 * np * sizeof (float));
          col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
          ind = (unsigned int *)malloc (nl * 2 * sizeof (unsigned int));
	}

      fclose (fp);
    }

  free (pl);

  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 255;

  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}

void cube_t::render () const
{
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  // GL_LINES : avec 3 points, on fait juste un segment (le dernier point n'est pas pris)
  // GL_LINE_LOOP : avec 3 points, on fait un triangle ferme
  // GL_LINE_STRIP : avec 3 points, on fait un triangle ouvert
//glDrawElements (GL_LINES, 3 * 1, GL_UNSIGNED_INT, (void *)(0 * sizeof (GLuint))); 
//glDrawElements (GL_LINES, 36 * 1, GL_UNSIGNED_INT, (void *)(0 * sizeof (GLuint)));
//glDrawElements (GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(3 * sizeof (GLuint)));
//glDrawElements (GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(0));
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, (void *)(0));
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);

}


void cube1_t::init ()
{
  ncol = use_alpha ? 4 : 3;
  nt = 12;
  np = 8;

  float * xyz = (float *)malloc (3 * np * sizeof (float));
  unsigned char * col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
  unsigned int * ind = (unsigned int *)malloc (nt * 3 * sizeof (unsigned int));

  float s = 1.0;

  xyz[0*3+0] = -s; xyz[0*3+1] = -s; xyz[0*3+2] = -s;
  xyz[1*3+0] = -s; xyz[1*3+1] = +s; xyz[1*3+2] = -s;
  xyz[2*3+0] = -s; xyz[2*3+1] = +s; xyz[2*3+2] = +s;
  xyz[3*3+0] = -s; xyz[3*3+1] = -s; xyz[3*3+2] = +s;
  xyz[4*3+0] = +s; xyz[4*3+1] = -s; xyz[4*3+2] = -s;
  xyz[5*3+0] = +s; xyz[5*3+1] = +s; xyz[5*3+2] = -s;
  xyz[6*3+0] = +s; xyz[6*3+1] = +s; xyz[6*3+2] = +s;
  xyz[7*3+0] = +s; xyz[7*3+1] = -s; xyz[7*3+2] = +s;


  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 20;

  for (int i = 0; i < np; i++)
  for (int j = 0; j < 3; j++)
    col[ncol*i+j] = 255 * ((s + xyz[i*3+j]) / (2 * s));

  ind[ 0] = 4; ind[ 1] = 5; ind[ 2] = 6;
  ind[ 3] = 4; ind[ 4] = 6; ind[ 5] = 7;

  ind[ 6] = 1; ind[ 7] = 0; ind[ 8] = 2;
  ind[ 9] = 2; ind[10] = 0; ind[11] = 3;

  ind[12] = 1; ind[13] = 2; ind[14] = 6;
  ind[15] = 1; ind[16] = 6; ind[17] = 5;

  ind[18] = 2; ind[19] = 3; ind[20] = 6;
  ind[21] = 3; ind[22] = 7; ind[23] = 6;

  ind[24] = 3; ind[25] = 0; ind[26] = 4;
  ind[27] = 3; ind[28] = 4; ind[29] = 7;

  ind[30] = 5; ind[31] = 4; ind[32] = 1;
  ind[33] = 1; ind[34] = 4; ind[36] = 0;

  
  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}

void cube_t::init ()
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  ncol = use_alpha ? 4 : 3;
  nt = 12;
  np = 8;

  float * xyz = (float *)malloc (3 * np * sizeof (float));
  unsigned char * col = (unsigned char *)malloc (np * ncol * sizeof (unsigned char));
  unsigned int * ind = (unsigned int *)malloc (nt * 3 * sizeof (unsigned int));

  float s = 1.0;

  xyz[0*3+0] = -s; xyz[0*3+1] = -s; xyz[0*3+2] = -s;
  xyz[1*3+0] = -s; xyz[1*3+1] = +s; xyz[1*3+2] = -s;
  xyz[2*3+0] = -s; xyz[2*3+1] = +s; xyz[2*3+2] = +s;
  xyz[3*3+0] = -s; xyz[3*3+1] = -s; xyz[3*3+2] = +s;
  xyz[4*3+0] = +s; xyz[4*3+1] = -s; xyz[4*3+2] = -s;
  xyz[5*3+0] = +s; xyz[5*3+1] = +s; xyz[5*3+2] = -s;
  xyz[6*3+0] = +s; xyz[6*3+1] = +s; xyz[6*3+2] = +s;
  xyz[7*3+0] = +s; xyz[7*3+1] = -s; xyz[7*3+2] = +s;


  for (int i = 0; i < np; i++)
  for (int j = 0; j < ncol; j++)
    col[ncol*i+j] = 120;

  for (int i = 0; i < np; i++)
  for (int j = 0; j < 3; j++)
    col[ncol*i+j] = 255 * ((s + xyz[i*3+j]) / (2 * s));

  ind[ 0] = 4; ind[ 1] = 5; ind[ 2] = 6;
  ind[ 3] = 4; ind[ 4] = 6; ind[ 5] = 7;

  ind[ 6] = 1; ind[ 7] = 0; ind[ 8] = 2;
  ind[ 9] = 2; ind[10] = 0; ind[11] = 3;

  ind[12] = 1; ind[13] = 2; ind[14] = 6;
  ind[15] = 1; ind[16] = 6; ind[17] = 5;

  ind[18] = 2; ind[19] = 3; ind[20] = 6;
  ind[21] = 3; ind[22] = 7; ind[23] = 6;

  ind[24] = 3; ind[25] = 0; ind[26] = 4;
  ind[27] = 3; ind[28] = 4; ind[29] = 7;

  ind[30] = 5; ind[31] = 4; ind[32] = 1;
  ind[33] = 1; ind[34] = 4; ind[36] = 0;

  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);

  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);

  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind, GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);
}


void world_t::init (const char * file)
{
  unsigned int * ind;
  float * xyz;
  unsigned char * col;
  ncol = use_alpha ? 4 : 3;

  glgrib_load (file, &np, &xyz, &col, &nt, &ind, use_alpha);
  
  def_from_xyz_col_ind (xyz, col, ind);

  free (ind);
  free (xyz);
  free (col);
}

void polyhedron_t::def_from_xyz_col_ind (const float * xyz, unsigned char * col, unsigned int * ind)
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * np * sizeof (float), xyz, GL_STATIC_DRAW);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
  
  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, ncol * np * sizeof (unsigned char), col, GL_STATIC_DRAW);
  glEnableVertexAttribArray (1); 
  glVertexAttribPointer (1, ncol, GL_UNSIGNED_BYTE, GL_TRUE, ncol * sizeof (unsigned char), NULL); 
  
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof (unsigned int), ind , GL_STATIC_DRAW);
}

void polyhedron_t::render () const
{
  glBindVertexArray (VertexArrayID);
  glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
}

void view_t::init (prog_t * prog)
{
  MatrixID = glGetUniformLocation (prog->programID, "MVP");
}

polyhedron_t::~polyhedron_t ()
{
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteBuffers (1, &colorbuffer);
  glDeleteBuffers (1, &vertexbuffer);
  glDeleteVertexArrays (1, &VertexArrayID);
}

prog_t::~prog_t ()
{
  glDeleteProgram (programID);
}

void prog_t::init ()
{
  programID = LoadShaders ();
}

void gl_init ()
{
  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS); 
}
  

