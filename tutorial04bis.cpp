#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "shader.hpp"

extern "C" void load (const char *, int *, float **, float **, unsigned int *, unsigned int **, int);

static float rc = 2.5, latc = 0., lonc = 0., fov = 50.;
static bool do_rotate = false;

static void key_callback (GLFWwindow * window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)
    {
      switch (key)
        {
          case GLFW_KEY_TAB:
            do_rotate = ! do_rotate;
            break;
          case GLFW_KEY_W:
            fov += 1.;
            break;
          case GLFW_KEY_Q:
            fov -= 1.;
            break;
          case GLFW_KEY_6:
            rc += 0.1;
            break;
          case GLFW_KEY_EQUAL:
            rc -= 0.1;
            break;
          case GLFW_KEY_SPACE:
            latc = 0.;
            lonc = 0.;
	    rc = 2.5;
	    break;
          case GLFW_KEY_UP:
            latc = latc + 5.;
            break;
          case GLFW_KEY_DOWN:
            latc = latc - 5.;
            break;
          case GLFW_KEY_LEFT:
            lonc = lonc - 5.;
            break;
          case GLFW_KEY_RIGHT:
            lonc = lonc + 5.;
            break;
	  default:
	    break;
	}
    }
}


int main (int argc, char * argv[])
{
  const char * file = argv[1];
  
  // Initialise GLFW
  if( !glfwInit() )
  {
  	fprintf( stderr, "Failed to initialize GLFW\n" );
  	getchar();
  	return -1;
  }
  
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* window = glfwCreateWindow (1024, 1024, file, NULL, NULL);

  if( window == NULL ){
  	fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
  	getchar();
  	glfwTerminate();
  	return -1;
  }
  glfwMakeContextCurrent(window);
  
  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
  	fprintf(stderr, "Failed to initialize GLEW\n");
  	getchar();
  	glfwTerminate();
  	return -1;
  }
  
  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  
  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  
  // Enable depth test
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  
  
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS); 
  
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  
  GLuint programID = LoadShaders ("TransformVertexShader.vertexshader", 
		                  "ColorFragmentShader.fragmentshader");
  
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  
  int np;
  unsigned int nt;
  unsigned int * ind;
  float * xyz, * col;
  int use_alpha = 1, ncol = use_alpha ? 4 : 3;
  load (file, &np, &xyz, &col, &nt, &ind, use_alpha);
  
  
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, 3 * np * sizeof(float), xyz, GL_STATIC_DRAW);
  

  GLuint colorbuffer;
  glGenBuffers(1, &colorbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  glBufferData(GL_ARRAY_BUFFER, ncol * np * sizeof(float), col, GL_STATIC_DRAW);
  
  GLuint elementbuffer;
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * nt * sizeof(unsigned int), ind , GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);

  glfwSetKeyCallback (window, key_callback);
  
  while (1)
    {
      if (do_rotate)
        lonc += 1.;
      float xc = rc * glm::cos (glm::radians (lonc)) * glm::cos (glm::radians (latc)), 
	    yc = rc * glm::sin (glm::radians (lonc)) * glm::cos (glm::radians (latc)),
	    zc = rc *                                  glm::sin (glm::radians (latc));
      glm::mat4 Projection = glm::perspective (glm::radians (fov), 1.0f / 1.0f, 0.1f, 100.0f);
      glm::mat4 View       = glm::lookAt (glm::vec3 (xc,yc,zc), 
		                          glm::vec3 (0,0,0), 
					  glm::vec3 (0,0,1));
      glm::mat4 Model      = glm::mat4 (1.0f);
      glm::mat4 MVP        = Projection * View * Model; 
  
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      glUseProgram (programID);
      
      glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
      
      glEnableVertexAttribArray (0);
      glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
      glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
      
      glEnableVertexAttribArray (1);
      glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
      glVertexAttribPointer (1, ncol, GL_FLOAT, GL_TRUE, ncol * sizeof (float), NULL);
      
      glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
      glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
      
      glDisableVertexAttribArray (0);
      glDisableVertexAttribArray (1);
      
      glfwSwapBuffers (window);
      glfwPollEvents ();
  
      if (glfwGetKey (window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        break;
      if (glfwWindowShouldClose (window) != 0) 
        break;
    } 
  
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &colorbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
  
  glfwTerminate();
  
  return 0;
}

