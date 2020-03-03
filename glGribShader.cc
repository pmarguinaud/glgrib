#include "glGribShader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glGribOpenGL.h"

#include <stdexcept>


static GLuint compileShader (const std::string & name, const std::string & code, GLuint type)
{
  int len;
  GLint res = GL_FALSE;
  GLuint id = glCreateShader (type);
  const char * str = code.c_str ();

  glShaderSource (id, 1, &str, nullptr);
  glCompileShader (id);

  glGetShaderiv (id, GL_COMPILE_STATUS, &res);
  glGetShaderiv (id, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (id, len, nullptr, &mess[0]);
      throw std::runtime_error (std::string ("Error compiling shader : ") + name + ", " + std::string (mess));
    }

  return id;
}

GLuint glGrib::LoadShader (const std::string & name,
		         const std::string & FragmentShaderCode,
		         const std::string & VertexShaderCode,
			 const std::string & GeometryShaderCode)
{
  GLuint VertexShaderID = compileShader (name, VertexShaderCode, GL_VERTEX_SHADER);
  GLuint FragmentShaderID = compileShader (name, FragmentShaderCode, GL_FRAGMENT_SHADER);

  bool geom = GeometryShaderCode != "";
  GLuint GeometryShaderID = 0;
  if (geom)
    GeometryShaderID = compileShader (name, GeometryShaderCode, GL_GEOMETRY_SHADER);

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
      glGetProgramInfoLog (ProgramID, len, nullptr, &mess[0]);
      throw std::runtime_error (std::string ("Error linking program : ") + name + ", " + std::string (mess));
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


