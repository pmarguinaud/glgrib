#include "glgrib_palette.h"
#include "glgrib_opengl.h"

#include <iostream>

typedef std::map<std::string,glgrib_palette> name2palette_t;
static name2palette_t name2palette;

void glgrib_palette::register_ (const glgrib_palette & p)
{
  name2palette.insert (std::pair<std::string, glgrib_palette>(name, p));
}

glgrib_palette & get_palette_by_name (const std::string & name)
{
  static glgrib_palette dummy;
  name2palette_t::iterator it = name2palette.find (name);
  if (it != name2palette.end ())
    return it->second;
  else
    return palette_cloud;
}

glgrib_palette palette_cold_hot
  (
     "cold_hot",
       0,   0, 255, 255,
     255, 255, 255, 255,
     255,   0,   0, 255 
  );

glgrib_palette palette_cloud
  (
     "cloud",
     255, 255, 255,   0,
     255, 255, 255, 255
  );


std::ostream & operator << (std::ostream &out, const glgrib_palette & p)
{
  out << "[";
  for (std::vector<glgrib_rgba>::const_iterator it = p.rgba.begin (); it != p.rgba.end (); it++)
    out << *it << ",";
  out << "]" << std::endl;
}

void glgrib_palette::setRGBA255 (GLuint programID) const
{
  float RGBA0[256][4];
  int n = rgba.size ();

  for (int j = 0; j < n-1; j++)
    {
      int j0 = j + 0;
      int j1 = j + 1;
      int i0 = (256 * j0) / (n - 1);
      int i1 = (256 * j1) / (n - 1);
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

  glUniform4fv (glGetUniformLocation (programID, "RGBA0"), 256*4, &RGBA0[0][0]);
}

