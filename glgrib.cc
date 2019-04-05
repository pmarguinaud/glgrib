#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "shader.h"

extern "C" void load (const char *, int *, float **, float **, unsigned int *, unsigned int **, int);

static bool do_rotate = false;

typedef struct view_t
{
  float rc = 2.5, latc = 0., lonc = 0., fov = 50.;
  GLuint MatrixID;
} view_t;

typedef struct prog_t
{
  GLuint programID;
} prog_t;

typedef struct obj_t
{
  GLuint VertexArrayID;
  GLuint vertexbuffer, colorbuffer, elementbuffer;
  int use_alpha = 1;
  unsigned int ncol, nt;
  int np;
} obj_t;

view_t View;

static void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
      switch (key)
        {
          case GLFW_KEY_TAB:
            do_rotate = ! do_rotate;
            break;
          case GLFW_KEY_W:
            View.fov += 1.;
            break;
          case GLFW_KEY_Q:
            View.fov -= 1.;
            break;
          case GLFW_KEY_6:
            View.rc += 0.1;
            break;
          case GLFW_KEY_EQUAL:
            View.rc -= 0.1;
            break;
          case GLFW_KEY_SPACE:
            View.latc = 0.;
            View.lonc = 0.;
	    View.rc = 2.5;
	    View.fov = 50.;
	    break;
          case GLFW_KEY_UP:
            View.latc = View.latc + 5.;
            break;
          case GLFW_KEY_DOWN:
            View.latc = View.latc - 5.;
            break;
          case GLFW_KEY_LEFT:
            View.lonc = View.lonc - 5.;
            break;
          case GLFW_KEY_RIGHT:
            View.lonc = View.lonc + 5.;
            break;
	  default:
	    break;
	}
    }
}


static
void display (const prog_t * prog, const obj_t * obj, const view_t * view)
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
  
  
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, obj->vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, obj->colorbuffer);
  glVertexAttribPointer (1, obj->ncol, GL_FLOAT, GL_TRUE, obj->ncol * sizeof (float), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, obj->elementbuffer);
  glDrawElements (GL_TRIANGLES, 3 * obj->nt, GL_UNSIGNED_INT, NULL);
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);
}

static
void obj_init (obj_t * obj, const char * file)
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

static
void view_init (prog_t * prog, view_t * view)
{
  view->MatrixID = glGetUniformLocation (prog->programID, "MVP");
}

static
void view_free (view_t * view)
{

}
      
static
void obj_free (obj_t * obj)
{
  glDeleteBuffers (1, &obj->vertexbuffer);
  glDeleteBuffers (1, &obj->colorbuffer);
  glDeleteVertexArrays (1, &obj->VertexArrayID);
}

static
void prog_free (prog_t * prog)
{
  glDeleteProgram (prog->programID);
}

static
void prog_init (prog_t * prog)
{
  prog->programID = LoadShaders ("TransformVertexShader.vertexshader", 
		                 "ColorFragmentShader.fragmentshader");
}

static
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
  
GLFWwindow * new_glfw_window (const char * file)
{
  GLFWwindow * window;

  if (! glfwInit ())
    {
      fprintf (stderr, "Failed to initialize GLFW\n");
      getchar ();
      return NULL;
    }
  
  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  window = glfwCreateWindow (1024, 1024, file, NULL, NULL);

  if (window == NULL)
    {
      fprintf (stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
      getchar ();
      glfwTerminate ();
      return NULL;
    }

  glfwMakeContextCurrent (window);
  
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) 
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      getchar ();
      glfwTerminate ();
      return NULL;
    }
  
  glfwSetInputMode (window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback (window, key_callback);

  return window;
}
  
static 
void free_glfw_window (GLFWwindow * window)
{
  glfwTerminate ();
}

int main (int argc, char * argv[])
{
  const char * file = argv[1];
  obj_t Obj;
  prog_t Prog;
  GLFWwindow * Window = new_glfw_window (file);
  
  gl_init ();
  prog_init (&Prog);
  obj_init (&Obj, file);
  view_init (&Prog, &View);
  
  while (1)
    {
      if (do_rotate)
        View.lonc += 1.;

      display (&Prog, &Obj, &View);

      glfwSwapBuffers (Window);
      glfwPollEvents ();
  
      if (glfwGetKey (Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        break;
      if (glfwWindowShouldClose (Window) != 0) 
        break;
    } 
  
  view_free (&View);
  obj_free (&Obj);
  prog_free (&Prog);
  
  free_glfw_window (Window);
  
  return 0;
}

