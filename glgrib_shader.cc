#include "glgrib_shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_opengl.h"

GLuint glgrib_load_shader (const std::string & FragmentShaderCode, const std::string & VertexShaderCode)
{
  int len;
  GLint res = GL_FALSE;
  GLuint VertexShaderID = glCreateShader (GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader (GL_FRAGMENT_SHADER);


  // Compile Vertex Shader
  const char * VertexShaderCode_str = VertexShaderCode.c_str ();
  glShaderSource (VertexShaderID, 1, &VertexShaderCode_str, NULL);
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
  const char * FragmentShaderCode_str = FragmentShaderCode.c_str ();
  glShaderSource (FragmentShaderID, 1, &FragmentShaderCode_str, NULL);
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


