
#version 330 core

in vec4 fragmentColor;

out vec4 color;

uniform bool dd;

void main()
{
  if (! dd)
    discard;
  color = vec4 (1.0f, 0.0f, 0.0f, 1.0f);
  color = fragmentColor;
}
