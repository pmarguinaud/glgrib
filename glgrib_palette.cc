#include "glgrib_palette.h"
#include "glgrib_opengl.h"
#include "glgrib_resolve.h"
#include "glgrib_sqlite.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>

const float glgrib_palette::defaultMin = std::numeric_limits<float>::max();
const float glgrib_palette::defaultMax = std::numeric_limits<float>::min();

typedef std::map<std::string,glgrib_palette> name2palette_t;
static name2palette_t name2palette;

glgrib_palette & glgrib_palette::register_ (const glgrib_palette & p)
{
  name2palette.insert (std::pair<std::string, glgrib_palette>(opts.name, p));
  name2palette_t::iterator it = name2palette.find (opts.name);
  return it->second;
}

static
glgrib_palette palette_white_black
  (
    "white_black",
      0,   0,   0,   0,
      0,   0,   0, 255,
    255, 255, 255, 255
  );

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

static
hsva_t rgba2hsva (const glgrib_option_color & rgba)
{
  float r = float (rgba.r) / 255.0f, g = float (rgba.g) / 255.0f;
  float b = float (rgba.b) / 255.0f, a = float (rgba.a) / 255.0f;

  float h, s, v;
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

static
glgrib_option_color hsva2rgba (const hsva_t & hsva)
{
  float h = hsva.h, s = hsva.s, v = hsva.v, a = hsva.a;

  int hi = int (h / 60.0f) % 6;
  float f = h / 60 - float (hi);

  float l = v * (1.0f - s);
  float m = v * (1.0f - f * s);
  float n = v * (1.0f - (1.0f - f) * s);

  float r, g, b;

  switch (hi)
    {
      case 0: r = v; g = n; b = l; break;
      case 1: r = m; g = v; b = l; break;
      case 2: r = l; g = v; b = n; break;
      case 3: r = l; g = m; b = v; break;
      case 4: r = n; g = l; b = v; break;
      case 5: r = v; g = l; b = m; break;
    }

  return glgrib_option_color (int (255.0f * r), int (255.0f * g), 
		              int (255.0f * b), int (255.0f * a));
}


glgrib_palette glgrib_palette::create 
        (const glgrib_options_palette & o,  
         float min, float max)
{
  glgrib_palette p;

  if (o.colors.size () != 0)
    {
      p.opts = o;

      if (p.opts.values.size () == 0)
        {
          if (p.opts.min == defaultMin)
            p.opts.min = min;
          if (p.opts.max == defaultMax)
            p.opts.max = max;
          int n = p.opts.colors.size () + 1;
          for (int i = 0; i < n; i++)
            p.opts.values.push_back (p.opts.min + ((p.opts.max - p.opts.min) * i) / (n-1));
        }

      if (p.opts.min == defaultMin)
        p.opts.min = o.values.front ();
      if (p.opts.max == defaultMax)
        p.opts.max = o.values.back ();

      p.rgba_mis = glgrib_option_color (0, 0, 0, 0);

      // Generate values
      if (p.opts.generate.on)
        {
          const int n = p.opts.generate.levels;
          std::vector<float> values;
	  const float d = (p.opts.values.back () - p.opts.values.front ()) / (n-1);

          for (int i = 0; i < n; i++)
            {
              float a = float (i) / float (n-1);
              values.push_back (a * p.opts.values.back () + (1.0f - a) * p.opts.values.front ());
	    }

	  p.opts.values = values;
        }
      
      // Generate rainbow (HSV rotation)
      if (p.opts.rainbow.on)
        {
          hsva_t hsva1 = rgba2hsva (p.opts.colors.front ());
          hsva_t hsva2 = rgba2hsva (p.opts.colors.back  ());

	  std::vector<glgrib_option_color> colors;

	  colors.push_back (p.opts.colors.front ());

          float h1 = hsva1.h, h2 = hsva2.h;

	  if (p.opts.rainbow.direct.on)
	    while (h2 < h1)
              h2 += 360.f;
	  else
            while (h2 > h1)
              h2 -= 360.0f;


	  const int n = p.opts.values.size ();
	  for (int i = 1; i < n-2; i++)
            {
              float c = float (i) / float (n-2);
              float h = fmod (h1 * (1.0f - c) + h2 * c, 360.0f);
              float s = hsva1.s * (1.0f - c) + hsva2.s * c;
	      float v = hsva1.v * (1.0f - c) + hsva2.v * c;
	      float a = hsva1.a * (1.0f - c) + hsva2.a * c;
	      hsva_t hsva (h, s, v, a);
	      glgrib_option_color color = hsva2rgba (hsva);
	      colors.push_back (color);
	    }

          colors.push_back (p.opts.colors.back ());

	  p.opts.colors = colors;
	}

      // Generate radient
      if (p.opts.values.size () == p.opts.colors.size ())
        {
          for (int i = 1, j = 0; i < 256; i++)
            {
              float val = (i-1) * (p.opts.max - p.opts.min) / 255 + p.opts.min;
              while (j < p.opts.values.size ())
                {
                  if (val < p.opts.values[j])
                    break;
                  j++;
                }
              if (j >= p.opts.values.size ())
                p.rgba.push_back (p.opts.colors.back ());
              else if (j == 0)
                p.rgba.push_back (p.opts.colors.front ());
              else
                {
                  glgrib_option_color c;
                  int ia = j-1, ib = j+0;
                  float vala = p.opts.values[ia], valb = p.opts.values[ib];
                  float b = (val - vala) / (valb - vala), a = 1.0 - b;
                  c.r = a * p.opts.colors[ia].r + b * p.opts.colors[ib].r;
                  c.g = a * p.opts.colors[ia].g + b * p.opts.colors[ib].g;
                  c.b = a * p.opts.colors[ia].b + b * p.opts.colors[ib].b;
                  c.a = a * p.opts.colors[ia].a + b * p.opts.colors[ib].a;
                  p.rgba.push_back (c);
                }
            }
        }
      // Discrete
      else
        {
          for (int i = 1, j = 0; i < 256; i++)
            {
              float val = (i-1) * (p.opts.max - p.opts.min) / 254 + p.opts.min;
              while (j < p.opts.values.size ())
                {
                  if (val < p.opts.values[j])
                    break;
                  j++;
                }
              if (j >= p.opts.values.size ())
                p.rgba.push_back (p.opts.colors.back ());
              else
                p.rgba.push_back (p.opts.colors[j-1]);
            }
         }
    }
  else
    {
      p = create_by_name (o.name);
      p.opts = o;
    }

  if (p.opts.min == defaultMin)
    p.opts.min = min;
  if (p.opts.max == defaultMax)
    p.opts.max = max;

  return p;
}

glgrib_palette & glgrib_palette::create_by_name (const std::string & name)
{
  name2palette_t::iterator it = name2palette.find (name);

  if (it != name2palette.end ())
    return it->second;

  glgrib_palette p;


  glgrib_sqlite db (glgrib_resolve ("glgrib.db"));
  glgrib_sqlite::stmt st = db.prepare ("SELECT hexa FROM PALETTES WHERE name = ?;");
  st.bindall (&name);


  bool found;
  std::string hexa;

  if ((found = st.fetch_row (&hexa)))
    {
      for (int i = 0; 8 * i + 7 < hexa.length (); i++)
        {
          int r, g, b, a;
          if (sscanf (&hexa[8*i], "%2x%2x%2x%2x", &r, &g, &b, &a) != 4)
            throw std::runtime_error ("Cannot parse hexa color");
          if (i == 0)
            p.rgba_mis = glgrib_option_color (r, g, b, a);
          else
            p.rgba.push_back (glgrib_option_color (r, g, b, a));
        }
      p.opts.name = name;
    }

    
  return found ? p.register_ (p) : palette_white_black;
}

glgrib_palette::glgrib_palette (std::ifstream & fh)
{
  std::string head;
  std::getline (fh, head);
  if (head[0] == '-')
    {
    }
  int r, g, b, a;
  fh >> r >> g >> b >> a;
  rgba_mis = glgrib_option_color (r, g, b, a);
  while (fh >> r >> g >> b >> a)
    rgba.push_back (glgrib_option_color (r, g, b, a));
}

glgrib_palette glgrib_palette::next (const glgrib_palette & p, float min, float max)
{
  name2palette_t::iterator it = name2palette.find (p.opts.name);
  if (it != name2palette.end ())
    it++;

  if (it == name2palette.end ())
    it = name2palette.begin ();
  
  glgrib_palette p1 = it->second;

  p1.opts.min = min;
  p1.opts.max = max;

  return p1;
}

glgrib_palette palette_zsdiff_big
  (
     -350.0f, +350.0f,
     "zsdiff_big",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cold_hot_temp
  (
     253.15, 313.15,
     "cold_hot_temp",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cold_hot
  (
//   253.15, 293.15,
     "cold_hot",
       0,   0,   0,   0,
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cloud
  (
     0., 100.,
     "cloud",
       0,   0,   0,   0,
     255, 255, 255,   0,
     255, 255, 255, 255
  );

glgrib_palette palette_cloud_auto
  (
     "cloud_auto",
       0,   0,   0,   0,
     255, 255, 255,   0,
     255, 255, 255, 255
  );


std::ostream & operator << (std::ostream & out, const glgrib_palette & p)
{
  out << p.rgba_mis << std::endl;
  out << "[";
  for (std::vector<glgrib_option_color>::const_iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
    out << *it << ",";
  out << "]" << std::endl;
  return out;
}

void glgrib_palette::getRGBA255 (float RGBA0[256][4]) const
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
          float w0 = (float)(i1-i) / (float)(i1-i0);
          float w1 = (float)(i-i0) / (float)(i1-i0);
          RGBA0[i][0] = (rgba[j0].r * w0 + rgba[j1].r * w1) / 255.;
          RGBA0[i][1] = (rgba[j0].g * w0 + rgba[j1].g * w1) / 255.;
          RGBA0[i][2] = (rgba[j0].b * w0 + rgba[j1].b * w1) / 255.;
          RGBA0[i][3] = (rgba[j0].a * w0 + rgba[j1].a * w1) / 255.;
        }
    }


}

void glgrib_palette::setRGBA255 (GLuint programID) const
{
  float RGBA0[256][4];
  getRGBA255 (RGBA0);
  glUniform4fv (glGetUniformLocation (programID, "RGBA0"), 256, &RGBA0[0][0]);
}

bool operator== (const glgrib_palette & p1, const glgrib_palette & p2)
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
 
bool operator!= (const glgrib_palette & p1, const glgrib_palette & p2)
{
  return ! (p1 == p2);
}
 
glgrib_option_color glgrib_palette::getColor (float val) const
{
  float RGBA0[256][4];
  getRGBA255 (RGBA0);
  int pal = std::max (1, std::min ((int)(1 + 254 * (val - opts.min) / (opts.max - opts.min)), 255));
  return glgrib_option_color (255 * RGBA0[pal][0], 255 * RGBA0[pal][1], 255 * RGBA0[pal][2], 255 * RGBA0[pal][3]);
}

int glgrib_palette::getColorIndex (float val) const
{
  int pal = std::max (1, std::min ((int)(1 + 254 * (val - opts.min) / (opts.max - opts.min)), 255));
  return pal;
}

