#version 330 core

in vec2 fragmentPos;
in vec2 fletterPos;
in float fletterVal;

out vec4 color;

uniform sampler2D texture;

uniform float xoff[30];
uniform float yoff[30];
uniform int nx = 0;
uniform int ny = 0;
uniform float aspect = 1.0;
uniform float scale = 1.0;
uniform vec4 color0;
uniform vec4 color1;

void main ()
{
  float dx = scale * aspect;
  float dy = scale;

  int ix = int (mod (fletterVal, nx));
  int iy = int (fletterVal / nx);

  float tx = (fragmentPos.x - fletterPos.x) / dx;
  float ty = (fragmentPos.y - fletterPos.y) / dy;

  tx = xoff[ix] + tx * (xoff[ix+1] - xoff[ix]);
  ty = yoff[iy] + ty * (yoff[iy+1] - yoff[iy]);

  vec4 col = texture2D (texture, vec2 (tx, ty));


  float a = col.r;

  bool usebg = color1.a > 0.0f;

  if (usebg)
    {
      color.r = color1.r * a + (1.0 - a) * color0.r;
      color.g = color1.g * a + (1.0 - a) * color0.g;
      color.b = color1.b * a + (1.0 - a) * color0.b;
      color.a = color1.a * a + (1.0 - a) * color0.a;
    }
  else
    {
      color.r = color0.r;
      color.g = color0.g;
      color.b = color0.b;
      color.a = 1 - a;
    }
}
