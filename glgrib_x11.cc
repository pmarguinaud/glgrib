#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glgrib_render.h"

using namespace glm;

#include "shader.h"

static bool do_rotate = false;

static 
void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
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
GLFWwindow * new_glfw_window (const char * file, int width, int height)
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
  
  window = glfwCreateWindow (width, height, file, NULL, NULL);

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

void x11_display (const char * file, int width, int height)
{
  obj_t Obj;
  prog_t Prog;
  GLFWwindow * Window = new_glfw_window (file, width, height);
  
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
}


