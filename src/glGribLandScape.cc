#include "glGribLandScape.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribBitmap.h"
#include "glGribPng.h"
#include "glGribResolve.h"

#include <iostream>
#include <algorithm>
#include <exception>

#include <cmath>

namespace glGrib
{

void Landscape::setupVertexAttributes () const
{
  Program * program = Program::load ("LANDSCAPE");

  const auto & geometry = getGeometry ();

  geometry->bindCoordinates (program->getAttributeLocation ("vertexLonLat"));
 
  auto attr = program->getAttributeLocation ("vertexHeight");
  if (d.heightbuffer)
    {   
      d.heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (attr);
      glVertexAttribPointer (attr, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }   
  else
    {   
      glDisableVertexAttribArray (attr);
      glVertexAttrib1f (attr, 0.0f);
    }   
 
  geometry->bindTriangles ();
}

namespace
{

bool endsWith (const std::string & str, const std::string & end)
{
  int i0 = std::max (str.length () - end.length (), (size_t)0);
  int i1 = end.length ();
  return end == str.substr (i0, i1);
}

}

void Landscape::setup (Loader * ld, const OptionsLandscape & o)
{
  if (! o.on)
    return;

  d.opts = o;

  setGeometry (Geometry::load (ld, d.opts.grid.path, d.opts.geometry,
                               d.opts.grid.number_of_latitudes));

  if (d.opts.color == OptionColor ("#00000000"))
    {
      BufferPtr<unsigned char> rgb;
      int w, h;
     
      if (endsWith (d.opts.path, ".png"))
        ReadPng (Resolve (d.opts.path), &w, &h, rgb);
      else if (endsWith (d.opts.path, ".bmp"))
        Bitmap (Resolve (d.opts.path), rgb, &w, &h);
      else
        throw std::runtime_error (std::string ("Unknown image format :") + d.opts.path);
     
      GLint sizemax;
      glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sizemax);
      if ((sizemax < w) || (sizemax < h))
        throw std::runtime_error (std::string ("Image is too large to be used as a texture :") + d.opts.path);
     
      d.texture = OpenGLTexturePtr (w, h, rgb);
    }

  if (d.opts.geometry.height.on)
    {
      const auto & geometry = getGeometry ();
      GeometryPtr geometry_height = Geometry::load (ld, d.opts.geometry.height.path, d.opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Landscape and height have different geometries"));

      const int size = geometry->getNumberOfPoints ();

      BufferPtr<float> data;
      FieldMetadata meta;

      ld->load (&data, d.opts.geometry.height.path, d.opts.geometry, &meta);

      d.heightbuffer = OpenGLBufferPtr<float> (size);

      auto height = d.heightbuffer->map ();

#pragma omp parallel for
      for (int jglo = 0; jglo < size; jglo++)
        height[jglo] = data[jglo] == meta.valmis ? 0.0f : d.opts.geometry.height.scale * ((*data)[jglo]-meta.valmin) / (meta.valmax - meta.valmin);

    }

  setReady ();
}

void Landscape::render (const View & view, const OptionsLight & light) const
{
  Program * program = Program::load ("LANDSCAPE");
  program->use ();

  view.setMVP (program);
  program->set (light);
  program->set ("isflat", d.opts.flat.on);

  if (d.opts.color == OptionColor ("#00000000"))
    {
      // the texture selection process is a bit obscure
      d.texture->bind (0);
      program->set ("texture", 0);
      program->set ("colored", 0);
    }
  else
    {
      program->set ("color0", d.opts.color);
      program->set ("colored", 1);
    }

  program->set ("scale0", d.opts.scale);

  if (d.opts.projection == "LONLAT")
    {
      program->set ("texproj", 0);
      program->set ("lonA", d.opts.lonlat.position.lon1 * deg2rad);
      program->set ("lonB", d.opts.lonlat.position.lon2 * deg2rad);
      program->set ("latA", d.opts.lonlat.position.lat1 * deg2rad);
      program->set ("latB", d.opts.lonlat.position.lat2 * deg2rad);
    }
  else if (d.opts.projection == "WEBMERCATOR")
    {
      const float a = 6378137;
      int L, IY0, IX0, IY1, IX1;

      std::string path = d.opts.path;

      size_t i = path.find_last_of ("/");
      if (i != std::string::npos)
        path = path.substr (i + 1);

      sscanf (path.c_str (), "WebMercator_%5d_%5d_%5d_%5d_%5d", &L, &IY0, &IX0, &IY1, &IX1);

      int N = 1;
      float F = twopi * a / 256;
      for (int i = 0; i < L; i++)
        N = N * 2;
      program->set ("texproj", 1);
      program->set ("F", F);
      program->set ("N", N);
      program->set ("IX0", IX0);
      program->set ("IY0", IY0);
      program->set ("IX1", IX1);
      program->set ("IY1", IY1);
    }

  VAID.bind ();

  if (d.opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  const auto & geometry = getGeometry ();
  geometry->renderTriangles ();

  if (d.opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  VAID.unbind ();

  view.delMVP (program);
}

}
