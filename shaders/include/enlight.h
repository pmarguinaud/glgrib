uniform vec4 RGBA0[256];
uniform float valmin, valmax;
uniform float palmin, palmax;

uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;
uniform float frac = 0.1;
uniform bool smoothed = true;


vec4 enlightFragment (vec3 fragmentPos, float fragmentVal, float missingFlag)
{
  if (missingFlag > 0.)
    discard;

  vec4 color;

  float total = 1.;

  if (light)
    {
      total = frac + (1.0 - frac) * max (dot (fragmentPos, lightDir), 0.0);
    }

  float val = valmin + (valmax - valmin) * (255.0 * fragmentVal - 1.0f) / 254.0f;
  float pal = max (1.0f, min (1.0f + 254.0f * (val - palmin) / (palmax - palmin), 255.0f));

  if (smoothed)
    {
      int pal0 = int (floor (pal)), pal1 = int (ceil (pal));
      bool same = pal0 == pal1;
      float a1 = same ? 1. : pal - pal0;
      float a0 = same ? 0. : pal1 - pal;
      color = RGBA0[pal0] * a0 + RGBA0[pal1] * a1;
    }
  else
    {
      color = RGBA0[int (pal)];
    }

  color.rgb = total * color.rgb;

  return color;
}

