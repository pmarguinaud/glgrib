#include "glgrib_palette.h"
#include "glgrib_opengl.h"

#include <iostream>
#include <stdio.h>

std::string palette_directory;

typedef std::map<std::string,glgrib_palette> name2palette_t;
static name2palette_t name2palette;

void glgrib_palette::register_ (const glgrib_palette & p)
{
  name2palette.insert (std::pair<std::string, glgrib_palette>(name, p));
}

static
glgrib_palette palette_white_black
  (
    "white_black",
      0,   0,   0,   0,
      0,   0,   0, 255,
    255, 255, 255, 255
  );

glgrib_palette & get_palette_by_name (const std::string & name)
{
  name2palette_t::iterator it = name2palette.find (name);

  if (it != name2palette.end ())
    return it->second;

//std::string pp = palette_directory + "/" + name + ".dat";
//std::iftream fh (pp);
//if (! fh.is_open ())

  return palette_white_black;
}

glgrib_palette & get_next_palette (const glgrib_palette & p)
{
  name2palette_t::iterator it = name2palette.find (p.name);
  if (it != name2palette.end ())
    it++;

  if (it == name2palette.end ())
    it = name2palette.begin ();
  
  return it->second;
}

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


std::ostream & operator << (std::ostream &out, const glgrib_palette & p)
{
  out << p.rgba_mis << std::endl;
  out << "[";
  for (std::vector<glgrib_rgba>::const_iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
    out << *it << ",";
  out << "]" << std::endl;
}

void glgrib_palette::setRGBA255 (GLuint programID) const
{
  float RGBA0[256][4];
  int n = rgba.size ();

  RGBA0[0][0] = rgba_mis.r; RGBA0[0][1] = rgba_mis.b;
  RGBA0[0][2] = rgba_mis.g; RGBA0[0][3] = rgba_mis.a;

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

  glUniform4fv (glGetUniformLocation (programID, "RGBA0"), 256, &RGBA0[0][0]);
}

