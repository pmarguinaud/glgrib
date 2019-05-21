#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

#include <string>

static glgrib_program PRG[GLGRIB_PROGRAM_SIZE] = 
{
  glgrib_program (  // 3 colors + alpha channel
R"CODE(
#version 330 core

in vec4 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = fragmentCol.a;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec4 vertexCol;

out vec4 fragmentCol;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
  fragmentCol.a = vertexCol.a;
}
)CODE"),

  glgrib_program (  // 3 colors 
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
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

layout(location = 0) in vec3 vertexPos;

uniform mat4 MVP;

void main()
{
  gl_Position =  MVP * vec4 (vertexPos, 1);
}
)CODE"),

  glgrib_program (  // 3 colors, flat
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;

void main()
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  vec3 pos;
  pos.x = x * r;
  pos.y = y * r;
  pos.z = z * r;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
}
)CODE"),

  glgrib_program (  // 3 colors, position, scale
R"CODE(
#version 330 core

in vec3 fragmentCol;

out vec4 color;

void main()
{
  color.r = fragmentCol.r;
  color.g = fragmentCol.g;
  color.b = fragmentCol.b;
  color.a = 255;
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexCol;

out vec3 fragmentCol;
uniform mat4 MVP;
uniform vec3 position0 = vec3 (0.0, 0.0, 0.0);
uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

void main()
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  vec3 pos;
  pos.x = scale0.x * x + position0.x;
  pos.y = scale0.y * y + position0.y;
  pos.z = scale0.z * z + position0.z;
  gl_Position =  MVP * vec4 (pos, 1);
  fragmentCol.r = vertexCol.r;
  fragmentCol.g = vertexCol.g;
  fragmentCol.b = vertexCol.b;
}
)CODE"),

  glgrib_program (  // gradient color + alpha, flat
R"CODE(
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;

out vec4 color;

uniform vec4 RGBA0[256];

uniform float valmin, valmax;
uniform float palmin, palmax;

uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;

void main ()
{
  float total = 1.;

  if (light)
    {
      total = 0.1 + 0.9 * max (dot (fragmentPos, lightDir), 0.0);
    }

  float val = valmin + (valmax - valmin) * (255.0 * fragmentVal - 1.0) / 254.0;

  int pal;
  if (val < valmin)
    {
      pal = 0;
      color.r = total * RGBA0[pal].r;
      color.g = total * RGBA0[pal].g;
      color.b = total * RGBA0[pal].b;
      color.a =         RGBA0[pal].a;
    }
  else
    {
      pal = max (1, min (int (1 + 254 * (val - palmin) / (palmax - palmin)), 255));
      color.r = total * RGBA0[pal].r;
      color.g = total * RGBA0[pal].g;
      color.b = total * RGBA0[pal].b;
      color.a =         RGBA0[pal].a;
    }

}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal;

out float fragmentVal;
out vec3 fragmentPos;

uniform mat4 MVP;
uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

void main ()
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  vec3 normedPos = vec3 (x * r, y * r, z * r);
  vec3 pos;
  pos.x = scale0.x * normedPos.x;
  pos.y = scale0.y * normedPos.y;
  pos.z = scale0.z * normedPos.z;
  gl_Position =  MVP * vec4 (pos, 1);

  fragmentVal = vertexVal;
  fragmentPos = normedPos;
}
)CODE"),

  glgrib_program (
R"CODE(
#version 330 core

in vec3 fragmentPos;
out vec4 color;

uniform sampler2D texture;
uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;

const float pi = 3.1415926;

void main ()
{
  float lon = (atan (fragmentPos.y, fragmentPos.x) / pi + 1.0) * 0.5;
  float lat = asin (fragmentPos.z) / pi + 0.5;

  vec4 col = texture2D (texture, vec2 (lon, lat));

  float total = 1.;

  if (light)
    {
      total = 0.1 + 0.9 * max (dot (fragmentPos, lightDir), 0.0);
    }

  color.r = total * col.r;
  color.g = total * col.g;
  color.b = total * col.b;
  color.a = 1.;
}
)CODE",
R"CODE(
#version 330 core

layout (location = 0) in vec3 vertexPos;

out vec3 fragmentPos;

uniform mat4 MVP;

const int XYZ=0;
const int POLAR_NORTH=1;
const int POLAR_SOUTH=2;
const int MERCATOR=3;
const int LATLON=4;
uniform int proj = 3;
uniform bool isflat = true;
const float pi = 3.1415926;
uniform float lon0 = 180.0; // Latitude of right handside


vec3 compPos ()
{
  return vec3 (0., 0., 0.);
}

void main()
{
  vec3 normedPos;

  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  normedPos.x = x * r;
  normedPos.y = y * r;
  normedPos.z = z * r;

  vec3 pos;

  switch (proj)
    {
      case XYZ:
        if (isflat)
          gl_Position =  MVP * vec4 (normedPos, 1);
        else
          gl_Position =  MVP * vec4 (vertexPos, 1);
        break;
      case POLAR_NORTH:
        gl_Position =  MVP * vec4 (0., normedPos.x / (+normedPos.z + 1.0), 
                                   normedPos.y / (+normedPos.z + 1.0), 1);
        break;      
      case POLAR_SOUTH:
        gl_Position =  MVP * vec4 (0., normedPos.x / (-normedPos.z + 1.0), 
                                   normedPos.y / (-normedPos.z + 1.0), 1);
        break;      
      case MERCATOR:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = log (tan (pi / 4. + lat / 2.)) / pi;
          gl_Position =  MVP * vec4 (0., X, Y, 1.);
        }
        break;
      case LATLON:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = lat / pi;
          gl_Position =  MVP * vec4 (0., X, Y, 1.);
        }
        break;
    }

  fragmentPos = normedPos;

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

