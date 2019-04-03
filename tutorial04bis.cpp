// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"

extern "C" void load (const char *, int *, float **, float **, int *, unsigned int **, int);

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
  
  window = glfwCreateWindow (1024, 1024, file, NULL, NULL);
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
  
  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );
  
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  
  glm::mat4 Projection = glm::perspective(glm::radians(50.0f), 1.0f / 1.0f, 0.1f, 100.0f);
  glm::mat4 View       = glm::lookAt(glm::vec3 (2.5,0,0), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 Model      = glm::mat4(1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  
  int np, nt;
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
  
  do{
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  	glUseProgram(programID);
  
  	glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
  
  	glEnableVertexAttribArray (0);
  	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  	glEnableVertexAttribArray (1);
  	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
  	glVertexAttribPointer (1, ncol, GL_FLOAT, GL_TRUE, ncol * sizeof (float), NULL);
  
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  	glDrawElements (GL_TRIANGLES, 3 * nt, GL_UNSIGNED_INT, NULL);
  
        glDisableVertexAttribArray (0);
        glDisableVertexAttribArray (1);
  
  	glfwSwapBuffers (window);
  	glfwPollEvents();
  
  } 
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
  	   glfwWindowShouldClose(window) == 0 );
  
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &colorbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
  
  glfwTerminate();
  
  return 0;
}

