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

using namespace glm;

#include "shader.h"

extern "C" void load (const char *, int *, float **, float **, unsigned int *, unsigned int **, int);

view_t View;

void display (const prog_t * prog, const obj_t * world, const obj_t * cube, const view_t * view)
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
  
  // World
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, world->vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, world->colorbuffer);
  glVertexAttribPointer (1, world->ncol, GL_FLOAT, GL_TRUE, world->ncol * sizeof (float), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, world->elementbuffer);
  glDrawElements (GL_TRIANGLES, 3 * world->nt, GL_UNSIGNED_INT, NULL);
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);


#ifdef UNDEF
  // Cube
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, cube->vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, cube->colorbuffer);
  glVertexAttribPointer (1, cube->ncol, GL_FLOAT, GL_TRUE, cube->ncol * sizeof (float), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, cube->elementbuffer);
  glDrawElements (GL_TRIANGLES, 3 * cube->nt, GL_UNSIGNED_INT, NULL);
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);
#endif


}

void cube_init (obj_t * obj)
{
  glGenVertexArrays (1, &obj->VertexArrayID);
  glBindVertexArray (obj->VertexArrayID);
  
  obj->ncol = obj->use_alpha ? 4 : 3;
  obj->nt = 1;
  obj->np = 3;

  float * xyz = (float *)malloc (3 * obj->np * sizeof (float));
  float * col = (float *)malloc (obj->np * obj->ncol * sizeof (float));
  unsigned int * ind = (unsigned int *)malloc (obj->nt * 3);

  xyz[0*3+0] = +0.0; xyz[0*3+1] = +2.0; xyz[0*3+2] = +0.0;
  xyz[1*3+0] = +0.0; xyz[1*3+1] = +0.0; xyz[1*3+2] = +2.0;
  xyz[2*3+0] = +0.0; xyz[2*3+1] = -2.0; xyz[2*3+2] = +0.0;

  for (int i = 0; i < obj->np; i++)
  for (int j = 0; j < obj->ncol; j++)
    col[obj->ncol*i+j] = 1.0;

  ind[0] = 0;
  ind[1] = 1;
  ind[2] = 2;

  
  glGenBuffers (1, &obj->vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, obj->vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * obj->np * sizeof (float), xyz, GL_STATIC_DRAW);
  

  glGenBuffers (1, &obj->colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, obj->colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, obj->ncol * obj->np * sizeof (float), col, GL_STATIC_DRAW);
  
  glGenBuffers (1, &obj->elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, obj->elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * obj->nt * sizeof (unsigned int), 
		ind , GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);
}

void world_init (obj_t * obj, const char * file)
{
  glGenVertexArrays (1, &obj->VertexArrayID);
  glBindVertexArray (obj->VertexArrayID);
  
  unsigned int * ind;
  float * xyz, * col;
  obj->ncol = obj->use_alpha ? 4 : 3;

  load (file, &obj->np, &xyz, &col, &obj->nt, &ind, obj->use_alpha);
  
  glGenBuffers (1, &obj->vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, obj->vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * obj->np * sizeof (float), xyz, GL_STATIC_DRAW);
  

  glGenBuffers (1, &obj->colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, obj->colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, obj->ncol * obj->np * sizeof (float), col, GL_STATIC_DRAW);
  
  glGenBuffers (1, &obj->elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, obj->elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * obj->nt * sizeof (unsigned int), 
		ind , GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);
}

void view_init (prog_t * prog, view_t * view)
{
  view->MatrixID = glGetUniformLocation (prog->programID, "MVP");
}

void view_free (view_t * view)
{

}
      
void obj_free (obj_t * obj)
{
  glDeleteBuffers (1, &obj->vertexbuffer);
  glDeleteBuffers (1, &obj->colorbuffer);
  glDeleteVertexArrays (1, &obj->VertexArrayID);
}

void prog_free (prog_t * prog)
{
  glDeleteProgram (prog->programID);
}

void prog_init (prog_t * prog)
{
  prog->programID = LoadShaders ();
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
  

