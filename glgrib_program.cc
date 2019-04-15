#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

#include <string>

static glgrib_program PRG[GLGRIB_PROGRAM_SIZE] = 
{
  glgrib_program (  // 3 colors + alpha channel
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

  glgrib_program (  // 3 colors 
R"CODE(
#version 330 core

in vec3 fragmentColor;

out vec4 color;

void main()
{
  color.r = fragmentColor.r;
  color.g = fragmentColor.g;
  color.b = fragmentColor.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPosition_modelspace, 1);
  fragmentColor.r = vertexColor.r;
  fragmentColor.g = vertexColor.g;
  fragmentColor.b = vertexColor.b;
}
)CODE"),

  glgrib_program ( // Fixed color
R"CODE(
#version 330 core

out vec4 color;

uniform vec3 color0; 

void main()
{
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 255;
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

  glgrib_program (  // 3 colors, flat
R"CODE(
#version 330 core

in vec3 fragmentColor;

out vec4 color;

void main()
{
  color.r = fragmentColor.r;
  color.g = fragmentColor.g;
  color.b = fragmentColor.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;
uniform mat4 MVP;

void main()
{
  float x = vertexPosition_modelspace.x;
  float y = vertexPosition_modelspace.y;
  float z = vertexPosition_modelspace.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  vec3 pos;
  pos.x = x * r;
  pos.y = y * r;
  pos.z = z * r;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentColor.r = vertexColor.r;
  fragmentColor.g = vertexColor.g;
  fragmentColor.b = vertexColor.b;
}
)CODE"),

  glgrib_program (  // 3 colors, position, scale
R"CODE(
#version 330 core

in vec3 fragmentColor;

out vec4 color;

void main()
{
  color.r = fragmentColor.r;
  color.g = fragmentColor.g;
  color.b = fragmentColor.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;
uniform mat4 MVP;
uniform vec3 position0 = vec3 (0.0, 0.0, 0.0);
uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

void main()
{
  float x = vertexPosition_modelspace.x;
  float y = vertexPosition_modelspace.y;
  float z = vertexPosition_modelspace.z;
  vec3 pos;
  pos.x = scale0.x * x + position0.x;
  pos.y = scale0.y * y + position0.y;
  pos.z = scale0.z * z + position0.z;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentColor.r = vertexColor.r;
  fragmentColor.g = vertexColor.g;
  fragmentColor.b = vertexColor.b;
}
)CODE"),

  glgrib_program (  // gradient color, flat
R"CODE(
#version 330 core

in vec3 fragmentColor;

out vec4 color;

void main()
{
  color.r = fragmentColor.r;
  color.g = fragmentColor.g;
  color.b = fragmentColor.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in float vertexColor;

out vec3 fragmentColor;
uniform mat4 MVP;

uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

void main()
{
  float x = vertexPosition_modelspace.x;
  float y = vertexPosition_modelspace.y;
  float z = vertexPosition_modelspace.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  vec3 pos;
  pos.x = scale0.x * x * r;
  pos.y = scale0.y * y * r;
  pos.z = scale0.z * z * r;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentColor.r = vertexColor;
  fragmentColor.g = 0;
  fragmentColor.b = 0;
}
)CODE"),


};

glgrib_program * glgrib_program_load (glgrib_program_kind kind)
{
  if (! PRG[kind].loaded)
    {
      PRG[kind].programID = glgrib_load_shader (PRG[kind].FragmentShaderCode, PRG[kind].VertexShaderCode);
      PRG[kind].matrixID = glGetUniformLocation (PRG[kind].programID, "MVP");
      PRG[kind].loaded = true;
    }
  return &PRG[kind];
}

glgrib_program::~glgrib_program ()
{
  if (loaded)
    glDeleteProgram (programID);
}


void glgrib_program::use () const
{
  glUseProgram (programID);
  if (! active)
    {
      glUseProgram (programID);
      active = true;
      for (int i = 0; i < GLGRIB_PROGRAM_SIZE; i++) 
        if (PRG[i].programID != programID)
          PRG[i].active = false;
    }
}

