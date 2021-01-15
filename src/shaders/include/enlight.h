
#include "palette.h"

uniform vec4 RGBAM = vec4 (0.0f, 0.0f, 0.0f, 0.0f);
uniform float valmin, valmax;
uniform float palmin, palmax;

uniform vec3 lightDir = vec3 (0., 1., 0.);
uniform vec3 lightCol = vec3 (1., 1., 1.);
uniform bool light = false;
uniform float frac = 0.1;
uniform bool smoothed = false;
uniform bool discrete = false;
uniform bool dinteger = false;


vec4 enlightFragment (vec3 fragmentPos, float fragmentVal, 
                      float missingFlag, float fragmentValFlat)
{
  if (missingFlag > 0.)
    discard;

  vec4 color;

  if ((discrete) && (abs (fragmentValFlat - fragmentVal) > 0.0001))
    {
      if (dinteger)
        {
          fragmentVal = round (fragmentVal);
        }
      else
        {
          if ((RGBAM.r == 0.0f) && (RGBAM.g == 0.0f) && (RGBAM.b == 0.0f) && (RGBAM.a == 0.0f))
            discard;
          color = RGBAM;
          return color;
        }
    }


  float total = 1.;

  if (light)
    {
      total = frac + (1.0 - frac) * max (dot (fragmentPos, lightDir), 0.0);
    }

  if (rgba_fixed)
    {
      //color = rgba_[int (fragmentValFlat)];
      float pal = fragmentVal;
      int pal0 = int (floor (pal)), pal1 = int (ceil (pal));
      bool same = pal0 == pal1;
      float a1 = same ? 1. : pal - pal0;
      float a0 = same ? 0. : pal1 - pal;
      if (smoothed)
        {
          // Smooth
          color = rgba_[pal0] * a0 + rgba_[pal1] * a1;
        }
      else
        {
          // Rough
          if (a1 > a0)
           color = rgba_[pal1];
          else
           color = rgba_[pal0];
        }
   
    }
  else
    {
      float val = unpack (fragmentVal, valmin, valmax);
     
      float rgba_size1 = float (rgba_size - 1);
      float rgba_size2 = float (rgba_size - 2);
      float pal = max (1.0f, 
                  min (1.0f + rgba_size2 * (val - palmin) / (palmax - palmin), 
                       rgba_size1));
     
      if (smoothed)
        {
          int pal0 = int (floor (pal)), pal1 = int (ceil (pal));
          bool same = pal0 == pal1;
          float a1 = same ? 1. : pal - pal0;
          float a0 = same ? 0. : pal1 - pal;
          color = rgba_[pal0] * a0 + rgba_[pal1] * a1;
        }
      else
        {
          color = rgba_[int (round (pal))];
        }
    }

  color.rgb = total * color.rgb;

  if (discrete)
    {
      if ((color.r == 0.0f) && (color.g == 0.0f) && (color.b == 0.0f) && (color.a == 0.0f))
        discard;
    }

  return color;
}

