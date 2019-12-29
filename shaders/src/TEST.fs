
#version 330 core

out vec4 color;

void main()
{
  int k = gl_PrimitiveID % 4;
  
  vec3 colors[4];
  colors[0] = vec3 (1.0f, 0.0f, 0.0f);
  colors[1] = vec3 (0.0f, 1.0f, 0.0f);
  colors[2] = vec3 (0.0f, 0.0f, 1.0f);
  colors[3] = vec3 (0.9f, 0.9f, 0.6f);
  
  color.rgb = colors[k];
  color.a   = 1.;
}
