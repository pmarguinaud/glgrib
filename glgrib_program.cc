#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

#include <string>


class prg_t
{
public:
  prg_t (const char * fsc, const char * vsc) : FragmentShaderCode (fsc), VertexShaderCode (vsc) { }
  ~prg_t ();
  const char * FragmentShaderCode = NULL;
  const char * VertexShaderCode = NULL;
  GLuint programID;
  bool loaded = false;
};

static prg_t PRG[] = 
{
  prg_t (
R"CODE(
#version 330 core

in vec4 fragmentColor;

out vec4 color;

void main()
{
  color.r = fragmentColor.r;
  color.g = fragmentColor.g;
  color.b = fragmentColor.b;
  color.a = fragmentColor.a;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 vertexColor;

out vec4 fragmentColor;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPosition_modelspace, 1);
  fragmentColor.r = vertexColor.r;
  fragmentColor.g = vertexColor.g;
  fragmentColor.b = vertexColor.b;
  fragmentColor.a = vertexColor.a;
}
)CODE"),

  prg_t (
R"CODE(
#version 330 core

out vec3 color;

void main()
{
  color.r = 255;
  color.g = 255;
  color.b = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPosition_modelspace, 1);
}
)CODE"),


};

GLuint glgrib_program (int kind)
{
  if (! PRG[kind].loaded)
    PRG[kind].programID = glgrib_load_shader (PRG[kind].FragmentShaderCode, PRG[kind].VertexShaderCode);
  return PRG[kind].programID;
}

prg_t::~prg_t ()
{
  if (loaded)
    glDeleteProgram (programID);
}

