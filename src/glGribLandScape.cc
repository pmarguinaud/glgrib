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
      texture = landscape.texture;
      opts    = landscape.opts;
      setupVertexAttributes ();
      setReady ();
   }
  return *this;
}

void glGrib::Landscape::setupVertexAttributes ()
{
  numberOfPoints = geometry->getNumberOfPoints ();
  numberOfTriangles = geometry->getNumberOfTriangles ();

  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  geometry->bindCoordinates (0);
 
  if (heightbuffer)
    {   
      heightbuffer->bind (GL_ARRAY_BUFFER);
      glEnableVertexAttribArray (1);
      glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }   
  else
    {   
      glDisableVertexAttribArray (1);
      glVertexAttrib1f (1, 0.0f);
    }   

 
  geometry->bindTriangles ();
  glBindVertexArray (0); 
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
  opts = o;

  geometry = glGrib::Geometry::load (ld, opts.geometry_path, opts.geometry, opts.number_of_latitudes);

  if (opts.color == glGrib::OptionColor ("#00000000"))
    {
      unsigned char * rgb;
      int w, h;
     
     
      if (endsWith (opts.path, ".png"))
        glGrib::ReadPng (glGrib::Resolve (opts.path), &w, &h, &rgb);
      else if (endsWith (opts.path, ".bmp"))
        glGrib::Bitmap (glGrib::Resolve (opts.path), &rgb, &w, &h);
      else
        throw std::runtime_error (std::string ("Unknown image format :") + opts.path);
     
      GLint sizemax;
      glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sizemax);
      if ((sizemax < w) || (sizemax < h))
        throw std::runtime_error (std::string ("Image is too large to be used as a texture :") + opts.path);
     
      texture = newGlgribOpenGLTexturePtr (w, h, rgb);
      delete [] rgb;
    }

  if (opts.geometry.height.on)
    {
      glGrib::GeometryPtr geometry_height = glGrib::Geometry::load (ld, opts.geometry.height.path, opts.geometry);

      if (! geometry_height->isEqual (*geometry))
        throw std::runtime_error (std::string ("Landscape and height have different geometries"));

      int size = geometry->getNumberOfPoints ();

      glGrib::FieldFloatBufferPtr data;
      glGrib::FieldMetadata meta;

      ld->load (&data, opts.geometry.height.path, opts.geometry, &meta);

      heightbuffer = newGlgribOpenGLBufferPtr (size * sizeof (float));

      float * height = static_cast<float *> (heightbuffer->map ());
#pragma omp parallel for
      for (int jglo = 0; jglo < size; jglo++)
        height[jglo] = (*data)[jglo] == meta.valmis ? 0.0f : opts.geometry.height.scale * ((*data)[jglo]-meta.valmin) / (meta.valmax - meta.valmin);

      heightbuffer->unmap (); 
    }


  setupVertexAttributes ();
  
  setReady ();
}

void glGrib::Landscape::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  glGrib::Program * program = glGrib::Program::load ("LANDSCAPE");
  program->use ();

  view.setMVP (program);
  program->set (light);
  program->set ("isflat", opts.flat.on);

  if (opts.color == glGrib::OptionColor ("#00000000"))
    {
      // the texture selection process is a bit obscure
      glActiveTexture (GL_TEXTURE0); 
      glBindTexture (GL_TEXTURE_2D, texture->id ());
      program->set ("texture", 0);
      program->set ("colored", 0);
    }
  else
    {
      program->set ("color0", opts.color);
      program->set ("colored", 1);
    }

  program->set ("scale0", opts.scale);

  if (opts.projection == "LONLAT")
    {
      program->set ("texproj", 0);
      program->set ("lonA", opts.lonlat.position.lon1 * deg2rad);
      program->set ("lonB", opts.lonlat.position.lon2 * deg2rad);
      program->set ("latA", opts.lonlat.position.lat1 * deg2rad);
      program->set ("latB", opts.lonlat.position.lat2 * deg2rad);
    }
  else if (opts.projection == "WEBMERCATOR")
    {
      const float a = 6378137;
      int L, IY0, IX0, IY1, IX1;

      std::string path = opts.path;

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

  glBindVertexArray (VertexArrayID);

  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);

  geometry->renderTriangles ();

  glBindVertexArray (0);

  if (opts.wireframe.on)
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  view.delMVP (program);
}

glGrib::Landscape::~Landscape ()
{
}





