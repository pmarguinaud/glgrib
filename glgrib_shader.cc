#include "glgrib_shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glgrib_opengl.h"

#include <stdexcept>


static GLuint compileShader (const std::string & code, GLuint type)
{
  int len;
  GLint res = GL_FALSE;
  GLuint id = glCreateShader (type);
  const char * str = code.c_str ();

  glShaderSource (id, 1, &str, NULL);
  glCompileShader (id);

  glGetShaderiv (id, GL_COMPILE_STATUS, &res);
  glGetShaderiv (id, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (id, len, NULL, &mess[0]);
      throw std::runtime_error (std::string ("Error compiling shader : ") + std::string (mess));
    }

  return id;
}

GLuint glgrib_load_shader (const std::string & FragmentShaderCode, 
		           const std::string & VertexShaderCode,
			   const std::string & GeometryShaderCode)
{
  GLuint VertexShaderID = compileShader (VertexShaderCode, GL_VERTEX_SHADER);
  GLuint FragmentShaderID = compileShader (FragmentShaderCode, GL_FRAGMENT_SHADER);

  bool geom = GeometryShaderCode != "";
  GLuint GeometryShaderID = 0;
  if (geom)
    GeometryShaderID = compileShader (GeometryShaderCode, GL_GEOMETRY_SHADER);

  // Link the program
  GLuint ProgramID = glCreateProgram ();
  glAttachShader (ProgramID, VertexShaderID);
  glAttachShader (ProgramID, FragmentShaderID);
  if (geom)
    glAttachShader (ProgramID, GeometryShaderID);

  glLinkProgram (ProgramID);
  
  // Check the program

  int len;
  GLint res = GL_FALSE;

  glGetProgramiv (ProgramID, GL_LINK_STATUS, &res);
  glGetProgramiv (ProgramID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetProgramInfoLog (ProgramID, len, NULL, &mess[0]);
      throw std::runtime_error (std::string ("Error linking program : ") + std::string (mess));
    }
  
  glDetachShader (ProgramID, VertexShaderID);
  glDetachShader (ProgramID, FragmentShaderID);
  if (geom)
    glDetachShader (ProgramID, GeometryShaderID);
  
  glDeleteShader (VertexShaderID);
  glDeleteShader (FragmentShaderID);

  if (geom)
    glDeleteShader (GeometryShaderID);
  
  return ProgramID;
}


