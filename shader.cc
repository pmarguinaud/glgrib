#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#include "shader.h"

GLuint LoadShaders ()
{
  int len;
  GLint res = GL_FALSE;
  GLuint VertexShaderID = glCreateShader (GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader (GL_FRAGMENT_SHADER);

  const char * FragmentShaderCode =
"#version 330 core\n"
"\n"
"// Interpolated values from the vertex shaders\n"
"in vec4 fragmentColor;\n"
"\n"
"// Ouput data\n"
"out vec4 color;\n"
"\n"
"void main(){\n"
"\n"
"        // Output color = color specified in the vertex shader, \n"
"        // interpolated between all 3 surrounding vertices\n"
"        color.r = fragmentColor.r;\n"
"        color.g = fragmentColor.g;\n"
"        color.b = fragmentColor.b;\n"
"        color.a = fragmentColor.a;\n"
"}\n";

  const char * VertexShaderCode = 
"#version 330 core\n"
"\n"
"// Input vertex data, different for all executions of this shader.\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec4 vertexColor;\n"
"\n"
"// Output data ; will be interpolated for each fragment.\n"
"out vec4 fragmentColor;\n"
"// Values that stay constant for the whole mesh.\n"
"uniform mat4 MVP;\n"
"\n"
"void main(){\n"
"\n"
"        // Output position of the vertex, in clip space : MVP * position\n"
"        gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
"\n"
"        // The color of each vertex will be interpolated\n"
"        // to produce the color of each fragment\n"
"        fragmentColor.r = vertexColor.r;\n"
"        fragmentColor.g = vertexColor.g;\n"
"        fragmentColor.b = vertexColor.b;\n"
"        fragmentColor.a = vertexColor.a;\n"
"}\n";


  // Compile Vertex Shader
  glShaderSource (VertexShaderID, 1, &VertexShaderCode , NULL);
  glCompileShader (VertexShaderID);

  glGetShaderiv (VertexShaderID, GL_COMPILE_STATUS, &res);
  glGetShaderiv (VertexShaderID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (VertexShaderID, len, NULL, &mess[0]);
      printf("%s\n", mess);
    }


  // Compile Fragment Shader
  glShaderSource (FragmentShaderID, 1, &FragmentShaderCode , NULL);
  glCompileShader (FragmentShaderID);

  glGetShaderiv (FragmentShaderID, GL_COMPILE_STATUS, &res);
  glGetShaderiv (FragmentShaderID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (FragmentShaderID, len, NULL, &mess[0]);
      printf("%s\n", mess);
    }

  // Link the program
  GLuint ProgramID = glCreateProgram ();
  glAttachShader (ProgramID, VertexShaderID);
  glAttachShader (ProgramID, FragmentShaderID);
  glLinkProgram (ProgramID);
  
  // Check the program
  glGetProgramiv (ProgramID, GL_LINK_STATUS, &res);
  glGetProgramiv (ProgramID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetProgramInfoLog (ProgramID, len, NULL, &mess[0]);
      printf("%s\n", mess);
    }
  
  
  glDetachShader (ProgramID, VertexShaderID);
  glDetachShader (ProgramID, FragmentShaderID);
  
  glDeleteShader (VertexShaderID);
  glDeleteShader (FragmentShaderID);
  
  return ProgramID;
}


