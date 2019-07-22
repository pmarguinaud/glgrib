#include "glgrib_program.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"

#include <string>
#include <math.h>
#include <glm/glm.hpp>


static const std::string projShaderInclude = 
R"CODE(
const int XYZ=0;
const int POLAR_NORTH=1;
const int POLAR_SOUTH=2;
const int MERCATOR=3;
const int LATLON=4;
uniform int proj = 0;
uniform bool isflat = true;
const float pi = 3.1415926;
uniform float lon0 = 180.0; // Latitude of right handside

vec3 compNormedPos (vec3 vertexPos)
{
  float x = vertexPos.x;
  float y = vertexPos.y;
  float z = vertexPos.z;
  float r = 1. / sqrt (x * x + y * y + z * z); 
  return vec3 (x * r, y * r, z * r);
}

vec3 compProjedPos (vec3 vertexPos, vec3 normedPos)
{
  vec3 pos;
  switch (proj)
    {
      case XYZ:
        if (isflat)
          pos = normedPos;
        else
          pos = vertexPos;
        break;
      case POLAR_NORTH:
        pos =  vec3 (0., normedPos.x / (+normedPos.z + 1.0), 
                     normedPos.y / (+normedPos.z + 1.0));
        break;      
      case POLAR_SOUTH:
        pos =  vec3 (0., normedPos.x / (-normedPos.z + 1.0), 
                     normedPos.y / (-normedPos.z + 1.0));
        break;      
      case MERCATOR:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = log (tan (pi / 4. + lat / 2.)) / pi;
          pos = vec3 (0., X, Y);
        }
        break;
      case LATLON:
        {
          float lat = asin (normedPos.z);
          float lon = mod (atan (normedPos.y, normedPos.x), 2 * pi);
          float X = (mod (lon - lon0 * pi / 180.0, 2 * pi) - pi) / pi;
          float Y = lat / pi;
          pos = vec3 (0., X, Y);
        }
        break;
    }

  return pos;
}
)CODE";


static const std::string scalePositionInclude = 
R"CODE(

uniform vec3 scale0 = vec3 (1.0, 1.0, 1.0);

vec3 scalePosition (vec3 pos, vec3 normedPos)
{
  if (proj == XYZ)
    {
      pos.x = scale0.x * normedPos.x;
      pos.y = scale0.y * normedPos.y;
      pos.z = scale0.z * normedPos.z;
    }
  else if (proj == POLAR_SOUTH)
    {
      pos.x = pos.x - (scale0.x - 1.0f);
    }
  else
    {
      pos.x = pos.x + (scale0.x - 1.0f);
    }

  return pos;
}

float scalingFactor (vec3 normedPos)
{
  if (proj == POLAR_SOUTH)
    {
      return 1.0 / (1.0 - normedPos.z);
    }
  else if (proj == POLAR_NORTH)
    {
      return 1.0 / (1.0 + normedPos.z);
    }
  else if (proj == MERCATOR)
    {
      return 1.0 / sqrt (1 - normedPos.z * normedPos.z);
    }
  return 1.0;
}


)CODE";


static const std::string enlightFragmentInclude = 
R"CODE(

uniform vec4 RGBA0[256];
uniform float valmin, valmax;
uniform float palmin, palmax;

uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;

vec4 enlightFragment (vec3 fragmentPos, float fragmentVal, float missingFlag)
{
  vec4 color;

  float total = 1.;

  if (light)
    {
      total = 0.1 + 0.9 * max (dot (fragmentPos, lightDir), 0.0);
    }

  float val = valmin + (valmax - valmin) * (255.0 * fragmentVal - 1.0) / 254.0;

  int pal;
//if (val < valmin)
  if (missingFlag > 0.)
    {
      discard;
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

  return color;
}
)CODE";


static glgrib_program PRG[glgrib_program::SIZE] = 
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

in float alpha;

out vec4 color;

uniform vec3 color0; 

void main()
{
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = alpha;

}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
out float alpha;


uniform mat4 MVP;
uniform bool do_alpha = false;
uniform float posmax = 0.97;

)CODE" + projShaderInclude + R"CODE(

void main()
{
  vec3 pos;

  alpha = 1.0;

  if (proj == XYZ)
    {
      pos = 1.005 * vertexPos;
    }
  else
    {
      vec3 normedPos = compNormedPos (vertexPos);
      pos = compProjedPos (vertexPos, normedPos);

      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          if (do_alpha)
            alpha = 0.0;
	}
      if (proj == LATLON)
      if ((pos.z > +0.49) || (pos.z < -0.49))
        alpha = 0.0;

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1.);

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

  glgrib_program (  // GRADIENT_FLAT_SCALE_SCALAR
R"CODE(
#version 330 core

in float fragmentVal;
in vec3 fragmentPos;
in float missingFlag;
out vec4 color;

)CODE" +
enlightFragmentInclude +
R"CODE(
void main ()
{
  color = enlightFragment (fragmentPos, fragmentVal, missingFlag);
}
)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal;

out float fragmentVal;
out vec3 fragmentPos;
out float missingFlag;

uniform mat4 MVP;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

void main ()
{
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  pos = scalePosition (pos, normedPos);

  gl_Position =  MVP * vec4 (pos, 1.);

  fragmentVal = vertexVal;
  fragmentPos = normedPos;
  missingFlag = vertexVal == 0 ? 1. : 0.;

}
)CODE"),

  glgrib_program (  // FLAT_TEX
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

)CODE" + projShaderInclude + R"CODE(

void main()
{
  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);
  gl_Position =  MVP * vec4 (pos, 1.);
  fragmentPos = normedPos;
}
)CODE"),

  glgrib_program (  // GRADIENT_FLAT_SCALE_VECTOR
R"CODE(
#version 330 core

out vec4 color;

uniform vec3 color0;

void main ()
{
  color.r = color0.r;
  color.g = color0.g;
  color.b = color0.b;
  color.a = 1.;
}

)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in float vertexVal_n;
layout(location = 2) in float vertexVal_d;

out vec3 fragmentPos;


uniform float valmin_n, valmax_n;
uniform float valmin_d, valmax_d;

uniform mat4 MVP;

)CODE" 
+ projShaderInclude 
+ scalePositionInclude 
+ R"CODE(

vec3 vprod (vec3 u, vec3 v)
{
  return vec3 (u.y * v.z - u.z * v.y, 
               u.z * v.x - u.x * v.z, 
               u.x * v.y - u.y * v.x);
}

const float deg2rad = pi / 180.0;

uniform float vscale = 0.01;

void main ()
{
  vec3 u = normalize (vec3 (-vertexPos.y, +vertexPos.x, 0.));
  vec3 v = vprod (vertexPos, u);

  bool defined = vertexVal_d != 0;
  vec3 pos;
  
  if (! defined)
    pos = vec3 (+0.0, +0.0, +0.0);
  else if (gl_VertexID == 0)
    pos = vec3 (+0.0, +0.0, +0.0);
  else if (gl_VertexID == 1)
    pos = vec3 (+1.0, +0.0, +0.0);
  else if (gl_VertexID == 2)
    pos = vec3 (+0.9, +0.1, +0.0);
  else if (gl_VertexID == 3)
    pos = vec3 (+0.9, -0.1, +0.0);
  else if (gl_VertexID == 4)
    pos = vec3 (+1.0, +0.0, +0.0);

  if (defined)
    {
      float N = valmin_n + (valmax_n - valmin_n) * (255.0 * vertexVal_n - 1.0) / 254.0;
      float D = valmin_d + (valmax_d - valmin_d) * (255.0 * vertexVal_d - 1.0) / 254.0;
      D = D * deg2rad;
      float X = vscale * N * cos (D) / valmax_n;
      float Y = vscale * N * sin (D) / valmax_n;
     
      pos = vertexPos + (pos.x * X - pos.y * Y) * u + (pos.x * Y + pos.y * X) * v;
     
      vec3 normedPos = compNormedPos (pos);
      vec3 projedPos = compProjedPos (pos, normedPos);
      pos = scalePosition (projedPos, normedPos);
    }

  gl_Position =  MVP * vec4 (pos, 1.);

}
)CODE"),

  glgrib_program (  // CONTOUR
R"CODE(
#version 330 core

in float alpha;
in float dist;
out vec4 color;

uniform vec3 color0;
uniform int N = 0;
uniform bool pattern[256];
uniform float length;
uniform bool dash;

void main ()
{
  if (alpha < 1.)
    discard;
  if(! dash)
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = 1.;
    }
  else
    {
      float r = mod (dist / length, 1.0f);
      int k = int (N * r);

      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;

      if (pattern[k])
        color.a = 1.;
      else
        color.a = 0.;

if(false){
      if (r > 0.5f)
        {
          color.r = 1.;
          color.g = 0.;
          color.b = 0.;
        }
      else
        {
          color.r = 0.;
          color.g = 1.;
          color.b = 0.;
        }
      color.a = 1.;
}
      
  }
}

)CODE",
R"CODE(
#version 330 core

layout(location = 0) in vec3 vertexPos0;
layout(location = 1) in vec3 vertexPos1;
layout(location = 2) in vec3 vertexPos2;
layout(location = 3) in float norm0;
layout(location = 4) in float norm1;
layout(location = 5) in float dist0;
layout(location = 6) in float dist1;


out float alpha;
out float dist;


uniform mat4 MVP;

)CODE"
+ projShaderInclude
+ scalePositionInclude
+ R"CODE(

uniform bool do_alpha = false;
uniform float posmax = 0.97;
uniform float width = 0.005;

void main ()
{
  vec3 vertexPos;
  vec3 t0 = normalize (vertexPos1 - vertexPos0);
  vec3 t1 = normalize (vertexPos2 - vertexPos1);

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    vertexPos = vertexPos0;
  else if ((gl_VertexID == 1) || (gl_VertexID == 3))
    vertexPos = vertexPos1;  

  vec3 p = normalize (vertexPos);
  vec3 n0 = cross (t0, p);
  vec3 n1 = cross (t1, p);

  float c = width / scalingFactor (p);

  if ((gl_VertexID >= 4) && (dot (cross (n0, n1), vertexPos) < 0.))
    c = 0.0;

  if (gl_VertexID == 2)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 3)
    vertexPos = vertexPos + c * n0;
  if (gl_VertexID == 4)
    vertexPos = vertexPos + c * normalize (n0 + n1);
  if (gl_VertexID == 5)
    vertexPos = vertexPos + c * n1;

  vec3 normedPos = compNormedPos (vertexPos);
  vec3 pos = compProjedPos (vertexPos, normedPos);

  alpha = min (norm0, norm1);

  if (proj == XYZ)
    {
      pos = scalePosition (pos, normedPos);
    }
  else
    {
      if ((proj == LATLON) || (proj == MERCATOR))
      if ((pos.y < -posmax) || (+posmax < pos.y))
        {
          pos.x = -0.1;
          if (do_alpha)
            alpha = 0.0;
	}
      if (proj == LATLON)
      if ((pos.z > +0.49) || (pos.z < -0.49))
        alpha = 0.0;

      if (proj == POLAR_SOUTH)
        pos.x = pos.x - 0.005;
      else
        pos.x = pos.x + 0.005;
    }

  gl_Position =  MVP * vec4 (pos, 1);

  if ((gl_VertexID == 0) || (gl_VertexID == 2))
    {
      dist = dist0;
    }
  else
    {
      dist = dist1;
    }



}
)CODE"),



};

void glgrib_program::compile ()
{
  if (loaded) 
    return;
  programID = glgrib_load_shader (FragmentShaderCode, VertexShaderCode);
  matrixID = glGetUniformLocation (programID, "MVP");
  loaded = true;
}

glgrib_program * glgrib_program_load (glgrib_program::kind kind)
{
  if (! PRG[kind].loaded)
    PRG[kind].compile ();
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
      for (int i = 0; i < glgrib_program::SIZE; i++) 
        if (PRG[i].programID != programID)
          PRG[i].active = false;
    }
}

void glgrib_program::setLight (const glgrib_options_light & light)
{
  int lightid = glGetUniformLocation (programID, "light");

  if (lightid != -1)
    {
      glUniform1i (lightid, light.on);
      if (light.on)
        {
          const double deg2rad = M_PI / 180.0;
          float coslon = cos (deg2rad * light.lon);
          float sinlon = sin (deg2rad * light.lon);
          float coslat = cos (deg2rad * light.lat);
          float sinlat = sin (deg2rad * light.lat);
          glm::vec3 lightDir = glm::vec3 (coslon * coslat, sinlon * coslat, sinlat);
          glUniform3fv (glGetUniformLocation (programID, "lightDir"), 1, &lightDir[0]);
        }
    }
}

void glgrib_program::set1f (const std::string & key, float val)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1f (id, val);
}

void glgrib_program::set1fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1fv (id, size, val);
}

void glgrib_program::set1iv (const std::string & key, const int * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1iv (id, size, val);
}

void glgrib_program::set1i (const std::string & key, int val)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform1i (id, val);
}

void glgrib_program::set3fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform3fv (id, size, val);
}

void glgrib_program::set4fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniform4fv (id, size, val);
}

void glgrib_program::setMatrix4fv (const std::string & key, const float * val, int size)
{
  int id = glGetUniformLocation (programID, key.c_str ());
  if (id != -1)
    glUniformMatrix4fv (id, size, GL_FALSE, val);
}



