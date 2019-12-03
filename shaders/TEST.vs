
#version 330 core

layout(location = 0) in vec3 vertexPos;

uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4 (vertexPos, 1);
}
