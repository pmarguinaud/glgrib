
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glgrib_opengl.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char * argv[])
{
  GLFWwindow * window;

  if (! glfwInit ()) 
    {   
      fprintf (stderr, "Failed to initialize GLFW\n");
      getchar (); 
      return -1;
    }   

  glfwWindowHint (GLFW_SAMPLES, 4);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow (256, 256, "", NULL, NULL);
  glfwMakeContextCurrent (window);
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK)
    {
      fprintf (stderr, "Failed to initialize GLEW\n");
      getchar ();
      glfwTerminate ();
      return -1;
    }

#define PR(x) \
  printf (" %-32s = %s\n", #x, glGetString (x))

  PR (GL_VENDOR);
  PR (GL_RENDERER);
  PR (GL_VERSION);
  PR (GL_EXTENSIONS);


  glfwTerminate ();
  return 0;
}
