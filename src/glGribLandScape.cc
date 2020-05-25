#include "glGribLandScape.h"
#include "glGribTrigonometry.h"
#include "glGribProgram.h"
#include "glGribBitmap.h"
#include "glGribPng.h"
#include "glGribResolve.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <exception>

#include <math.h>

glGrib::Landscape & glGrib::Landscape::operator= (const glGrib::Landscape & landscape)
{
  clear ();
  if ((this != &landscape) && landscape.isReady ())
    {
      glGrib::World::operator= (landscape);
      d = landscape.d;
      VAID = landscape.VAID;
      setReady ();
   }
  return *this;
}

void glGrib::Landscape::setupVertexAttributes () const
{
  geometry->bindCoordinates (0);
 
  if (d.heightbuffer)
    {   
      d.heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (1);
      glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }   
  else
    {   
      glDisableVertexAttribArray (1);
      glVertexAttrib1f (1, 0.0f);
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

void glGrib::Landscape::setup (glGrib::Loader * ld, const glGrib::OptionsLandscape & o)
{
  d.opts = o;

  geometry = glGrib::Geometry::load (ld, d.opts.geometry_path, d.opts.geometry, 
                                     d.opts.number_of_latitudes);

  if (d.opts.color == glGrib::OptionColor ("#00000000"))
    {
      unsigned char * rgb;
      int w, h;
     
     
      if (endsWith (d.opts.path, ".png"))
        glGrib::ReadPng (glGrib::Resolve (d.opts.path), &w, &h, &rgb);
      else if (endsWith (d.opts.path, ".bmp"))
        glGrib::Bitmap (glGrib::Resolve (d.opts.path), &rgb, &w, &h);
      else
        throw std::runtime_error (std::string ("Unknown image format :") + d.opts.path);
     
      GLint sizemax;
      glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sizemax);
      if ((sizemax < w) || (sizemax < h))
        throw std::runtime_error (std::string ("Image is too large to be used as a texture :") + d.opts.path);
     
      d.texture = newGlgribOpenGLTexturePtr (w, h, rgb);
      delete [] rgb;
    }

  if (d.opts.geometry.height.on)
    {
      glGrib::GeometryPtr geometry_height = glGrib::Geometry::load (ld, d.opts.geometry.height.path, d.opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Landscape and height have different geometries"));

      int size = geometry->getNumberOfPoints ();

      glGrib::FieldFloatBufferPtr data;
      glGrib::FieldMetadata meta;

      ld->load (&data, d.opts.geometry.height.path, d.opts.geometry, &meta);

      d.heightbuffer = newGlgribOpenGLBufferPtr (size * sizeof (float));

      float * height = static_cast<float *> (d.heightbuffer->map ());
#pragma omp parallel for
      for (int jglo = 0; jglo < size; jglo++)
        height[jglo] = (*data)[jglo] == meta.valmis ? 0.0f : d.opts.geometry.height.scale * ((*data)[jglo]-meta.valmin) / (meta.valmax - meta.valmin);

      d.heightbuffer->unmap (); 
    }


  setReady ();
}

void glGrib::Landscape::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glGrib::Program * program = glGrib::Program::load ("LANDSCAPE");
  program->use ();

  view.setMVP (program);
  program->set (light);
  program->set ("isflat", d.opts.flat.on);

  if (d.opts.color == glGrib::OptionColor ("#00000000"))
    {
      // the texture selection process is a bit obscure
      glActiveTexture (GL_TEXTURE0); 
      glBindTexture (GL_TEXTURE_2D, d.texture->id ());
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

  VAID.bindAuto ();

  if (d.opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  geometry->renderTriangles ();

  if (d.opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  VAID.unbind ();

  view.delMVP (program);
}

glGrib::Landscape::~Landscape ()
{
}





