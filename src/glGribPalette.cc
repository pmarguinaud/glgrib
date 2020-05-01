#include "glGribPalette.h"
#include "glGribOpenGL.h"
#include "glGribResolve.h"
#include "glGribSQLite.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>



namespace
{

class hsva_t
{
public:
  hsva_t (float _h, float _s, float _v, float _a) 
     : h (_h), s (_s), v (_v), a (_a) {}
  float h, s, v, a;
  friend std::ostream & operator << (std::ostream & os, const hsva_t & hsva)
  {
    os << " h = " << hsva.h << " s = " << hsva.s 
       << " v = " << hsva.v << " a = " << hsva.a 
       << std::endl;
    return os;
  }
};


hsva_t rgba2hsva (const glGrib::OptionColor & rgba)
{
  float r = float (rgba.r) / 255.0f, g = float (rgba.g) / 255.0f;
  float b = float (rgba.b) / 255.0f, a = float (rgba.a) / 255.0f;

  float h = 0.0f, s, v;
  float min = std::min (std::min (r, g), b);
  float max = std::max (std::max (r, g), b);

  if (max == min)
    h = 0.0f;
  else if (max == r)
    h = fmod (60.0f * (g - b) / (max - min) + 360.0f, 360.0f);
  else if (max == g)
    h =       60.0f * (b - r) / (max - min) + 120.0f;
  else if (max == b)
    h =       60.0f * (r - g) / (max - min) + 240.0f;

  if (max == 0.0f)
    s = 0.;
  else
    s = 1.0f - min / max;
  
  v = max;

  return hsva_t (h, s, v, a);
}


glGrib::OptionColor hsva2rgba (const hsva_t & hsva)
{
  float h = hsva.h, s = hsva.s, v = hsva.v, a = hsva.a;

  int hi = int (h / 60.0f) % 6;
  float f = h / 60 - float (hi);

  float l = v * (1.0f - s);
  float m = v * (1.0f - f * s);
  float n = v * (1.0f - (1.0f - f) * s);

  float r = 0.0f, g = 0.0f, b = 0.0f;

  switch (hi)
    {
      case 0: r = v; g = n; b = l; break;
      case 1: r = m; g = v; b = l; break;
      case 2: r = l; g = v; b = n; break;
      case 3: r = l; g = m; b = v; break;
      case 4: r = n; g = l; b = v; break;
      case 5: r = v; g = l; b = m; break;
    }

  return glGrib::OptionColor (int (255.0f * r), int (255.0f * g), 
		              int (255.0f * b), int (255.0f * a));
}

}

glGrib::Palette::Palette (const glGrib::OptionsPalette & o,  
                          float min, float max)
{
  createByOpts (o, min, max);
}

void glGrib::Palette::createByOpts (const glGrib::OptionsPalette & o,  
                                    float min, float max)
{
  if (o.colors.size () != 0)
    {
      opts = o;

      if (opts.values.size () == 0)
        {
          if (opts.min == defaultMin ())
            opts.min = min;
          if (opts.max == defaultMax ())
            opts.max = max;
          int n = opts.colors.size () + 1;
          for (int i = 0; i < n; i++)
            opts.values.push_back (opts.min + ((opts.max - opts.min) * i) / (n-1));
        }

      if (opts.min == defaultMin ())
        opts.min = o.values.front ();
      if (opts.max == defaultMax ())
        opts.max = o.values.back ();

      rgba_mis = glGrib::OptionColor (0, 0, 0, 0);

      // Generate values
      if (opts.generate.on)
        {
          const int n = opts.generate.levels;
          std::vector<float> values;

          for (int i = 0; i < n; i++)
            {
              float a = float (i) / float (n-1);
              values.push_back (a * opts.values.back () + (1.0f - a) * opts.values.front ());
	    }

	  opts.values = values;
        }
      
      // Generate rainbow (HSV rotation)
      if (opts.rainbow.on)
        {
          hsva_t hsva1 = rgba2hsva (opts.colors.front ());
          hsva_t hsva2 = rgba2hsva (opts.colors.back  ());

	  std::vector<glGrib::OptionColor> colors;

	  colors.push_back (opts.colors.front ());

          float h1 = hsva1.h, h2 = hsva2.h;

	  if (opts.rainbow.direct.on)
	    while (h2 < h1)
              h2 += 360.f;
	  else
            while (h2 > h1)
              h2 -= 360.0f;


	  const int n = opts.values.size ();
	  for (int i = 1; i < n-2; i++)
            {
              float c = float (i) / float (n-2);
              float h = fmod (h1 * (1.0f - c) + h2 * c, 360.0f);
              float s = hsva1.s * (1.0f - c) + hsva2.s * c;
	      float v = hsva1.v * (1.0f - c) + hsva2.v * c;
	      float a = hsva1.a * (1.0f - c) + hsva2.a * c;
	      hsva_t hsva (h, s, v, a);
	      glGrib::OptionColor color = hsva2rgba (hsva);
	      colors.push_back (color);
	    }

          colors.push_back (opts.colors.back ());

	  opts.colors = colors;
	}

      // Generate gradient
      if (opts.values.size () == opts.colors.size ())
        {
          size_t j = 0;
          for (int i = 1; i < 256; i++)
            {
              float val = (i-1) * (opts.max - opts.min) / 255 + opts.min;
              while (j < opts.values.size ())
                {
                  if (val < opts.values[j])
                    break;
                  j++;
                }
              if (j >= opts.values.size ())
                rgba.push_back (opts.colors.back ());
              else if (j == 0)
                rgba.push_back (opts.colors.front ());
              else
                {
                  glGrib::OptionColor c;
                  int ia = j-1, ib = j+0;
                  float vala = opts.values[ia], valb = opts.values[ib];
                  float b = (val - vala) / (valb - vala), a = 1.0 - b;
                  c.r = a * opts.colors[ia].r + b * opts.colors[ib].r;
                  c.g = a * opts.colors[ia].g + b * opts.colors[ib].g;
                  c.b = a * opts.colors[ia].b + b * opts.colors[ib].b;
                  c.a = a * opts.colors[ia].a + b * opts.colors[ib].a;
                  rgba.push_back (c);
                }
            }
        }
      // Discrete
      else
        {
          size_t j = 0;
          for (int i = 1; i < 256; i++)
            {
              float val = (i-1) * (opts.max - opts.min) / 254 + opts.min;
              while (j < opts.values.size ())
                {
                  if (val < opts.values[j])
                    break;
                  j++;
                }
              if (j >= opts.values.size ())
                rgba.push_back (opts.colors.back ());
              else
                rgba.push_back (opts.colors[j-1]);
            }
         }
    }
  else
    {
      createByName (o.name, min, max);
      opts = o;
    }

  if (opts.min == defaultMin ())
    opts.min = min;
  if (opts.max == defaultMax ())
    opts.max = max;

}


void glGrib::Palette::createByName (const std::string & name, float min, float max)
{
  glGrib::SQLite db (glGrib::Resolve ("glGrib.db"));
  glGrib::SQLite::stmt st = db.prepare ("SELECT hexa FROM PALETTES WHERE name = ?;");
  st.bindall (&name);


  bool found;
  std::string hexa;

  if ((found = st.fetchRow (&hexa)))
    {
      for (size_t i = 0; 8 * i + 7 < hexa.length (); i++)
        {
          int r, g, b, a;
          if (sscanf (&hexa[8*i], "%2x%2x%2x%2x", &r, &g, &b, &a) != 4)
            throw std::runtime_error ("Cannot parse hexa color");
          if (i == 0)
            rgba_mis = glGrib::OptionColor (r, g, b, a);
          else
            rgba.push_back (glGrib::OptionColor (r, g, b, a));
        }
      opts.name = name;
    }
  else
    {

      auto defp = [&] (const std::vector<OptionColor> & colors,
                       const float min, const float max)
      {
        glGrib::OptionsPalette opts;
        opts.colors = colors;
        for (size_t i = 0; i < colors.size (); i++)
          opts.values.push_back (min + (max - min) * static_cast<float>(i)
                               / static_cast<float>(colors.size ()));
        createByOpts (opts, min, max);
        opts.name = name;
      };

      if (name == "cold_hot_temp")
        defp (std::vector<OptionColor>{OptionColor ("blue"), OptionColor ("white"), 
                                       OptionColor ("red")}, 253.15, 313.15);
      else if (name == "cold_hot")
        defp (std::vector<OptionColor>{OptionColor ("blue"), OptionColor ("white"), 
                                       OptionColor ("red")}, min, max);
      else if (name == "cloud")
        defp (std::vector<OptionColor>{OptionColor ("#ffffff00"), OptionColor ("white")}, 0, 100);
      else if (name == "cloud_auto")
        defp (std::vector<OptionColor>{OptionColor ("#ffffff00"), OptionColor ("white")}, min, max);
      else
        defp (std::vector<OptionColor>{OptionColor ("black"), OptionColor ("white")}, min, max);

       

    }

}

void glGrib::Palette::getRGBA255 (float RGBA0[256][4]) const
{
  int n = rgba.size ();

  RGBA0[0][0] = rgba_mis.r; RGBA0[0][1] = rgba_mis.b;
  RGBA0[0][2] = rgba_mis.g; RGBA0[0][3] = rgba_mis.a;

#pragma omp parallel for
  for (int j = 0; j < n-1; j++)
    {
      int j0 = j + 0;
      int j1 = j + 1;
      int i0 = 1 + (255 * j0) / (n - 1);
      int i1 = 1 + (255 * j1) / (n - 1);
      for (int i = i0; i < i1; i++)
        {
          float w0 = static_cast<float> (i1-i) / static_cast<float> (i1-i0);
          float w1 = static_cast<float> (i-i0) / static_cast<float> (i1-i0);
          RGBA0[i][0] = (rgba[j0].r * w0 + rgba[j1].r * w1) / 255.;
          RGBA0[i][1] = (rgba[j0].g * w0 + rgba[j1].g * w1) / 255.;
          RGBA0[i][2] = (rgba[j0].b * w0 + rgba[j1].b * w1) / 255.;
          RGBA0[i][3] = (rgba[j0].a * w0 + rgba[j1].a * w1) / 255.;
        }
    }


}

void glGrib::Palette::setRGBA255 (GLuint programID) const
{
  float RGBA0[256][4];
  getRGBA255 (RGBA0);
  glUniform4fv (glGetUniformLocation (programID, "RGBA0"), 256, &RGBA0[0][0]);
}

namespace glGrib
{

bool operator== (const glGrib::Palette & p1, const glGrib::Palette & p2)
{
  if (p1.rgba_mis != p2.rgba_mis)
    return false;
  if (p1.opts.min != p2.opts.min)
    return false;
  if (p1.opts.max != p2.opts.max)
    return false;
  if (p1.rgba != p2.rgba)
    return false;
  return true;
}
 
bool operator!= (const glGrib::Palette & p1, const glGrib::Palette & p2)
{
  return ! (p1 == p2);
}

}
 
glGrib::OptionColor glGrib::Palette::getColor (float val) const
{
  float RGBA0[256][4];
  getRGBA255 (RGBA0);
  int pal = std::max (1, std::min (static_cast<int>(1 + 254 * (val - opts.min) / (opts.max - opts.min)), 255));
  return glGrib::OptionColor (255 * RGBA0[pal][0], 255 * RGBA0[pal][1], 255 * RGBA0[pal][2], 255 * RGBA0[pal][3]);
}

int glGrib::Palette::getColorIndex (float val) const
{
  int pal = std::max (1, std::min (static_cast<int>(1 + 254 * (val - opts.min) / (opts.max - opts.min)), 255));
  return pal;
}

