#include "glgrib_landscape.h"
#include "glgrib_program.h"
#include "glgrib_bitmap.h"
#include "glgrib_png.h"
#include "glgrib_resolve.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <exception>

#include <math.h>

glgrib_landscape & glgrib_landscape::operator= (const glgrib_landscape & landscape)
{
  clear ();
  if ((this != &landscape) && landscape.isReady ())
    {
      glgrib_world::operator= (landscape);
      texture = landscape.texture;
      opts    = landscape.opts;
      setupVertexAttributes ();
      setReady ();
   }
  return *this;
}

void glgrib_landscape::setupVertexAttributes ()
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
      glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    }   
  else
    {   
      glDisableVertexAttribArray (1);
      glVertexAttrib1f (1, 0.0f);
    }   

 
  geometry->bindTriangles ();
  glBindVertexArray (0); 
}

static bool endsWith (const std::string & str, const std::string & end)
{
  int i0 = std::max (str.length () - end.length (), (size_t)0);
  int i1 = end.length ();
  return end == str.substr (i0, i1);
}

void glgrib_landscape::setup (glgrib_loader * ld, const glgrib_options_landscape & o)
{
  opts = o;

  unsigned char * rgb;
  int w, h;

  geometry = glgrib_geometry::load (ld, opts.geometry_path, opts.geometry, opts.number_of_latitudes);

  if (endsWith (opts.path, ".png"))
    glgrib_read_png (glgrib_resolve (opts.path), &w, &h, &rgb);
  else if (endsWith (opts.path, ".bmp"))
    glgrib_bitmap (glgrib_resolve (opts.path), &rgb, &w, &h);
  else
    throw std::runtime_error (std::string ("Unknown image format :") + opts.path);

  GLint sizemax;
  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &sizemax);
  if ((sizemax < w) || (sizemax < h))
    throw std::runtime_error (std::string ("Image is too large to be used as a texture :") + opts.path);

  texture = new_glgrib_opengl_texture_ptr (w, h, rgb);
  delete [] rgb;

  if (opts.geometry.height.on)
    {
      int size = geometry->getNumberOfPoints ();

      glgrib_field_float_buffer_ptr data;
      glgrib_field_metadata meta;

      ld->load (&data, opts.geometry.height.path, opts.geometry, &meta);

      heightbuffer = new_glgrib_opengl_buffer_ptr (size * sizeof (float));

      float * height = (float *)heightbuffer->map (); 
#pragma omp parallel for
      for (int jglo = 0; jglo < size; jglo++)
        height[jglo] = (*data)[jglo] == meta.valmis ? 0.0f : opts.geometry.height.scale * ((*data)[jglo]-meta.valmin) / (meta.valmax - meta.valmin);

      heightbuffer->unmap (); 
    }


  setupVertexAttributes ();
  
  setReady ();
}

void glgrib_landscape::render (const glgrib_view & view, const glgrib_options_light & light) const
{
  const float deg2rad = M_PI / 180.0;

  glgrib_program * program = glgrib_program_load (glgrib_program::FLAT_TEX);
  program->use ();

  view.setMVP (program);
  program->setLight (light);
  program->set1i ("isflat", opts.flat.on);

  // the texture selection process is a bit obscure
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture->id ());
  program->set1i ("texture", 0);

  
  if (opts.projection == "LONLAT")
    {
      program->set1i ("texproj", 0);
      program->set1f ("lonA", opts.lonlat.position.lon1 * deg2rad);
      program->set1f ("lonB", opts.lonlat.position.lon2 * deg2rad);
      program->set1f ("latA", opts.lonlat.position.lat1 * deg2rad);
      program->set1f ("latB", opts.lonlat.position.lat2 * deg2rad);
    }
  else if (opts.projection == "WEBMERCATOR")
    {
      const float a = 6378137;
      int L, IY0, IX0, IY1, IX1;

      std::string path = opts.path;

      int i = path.find_last_of ("/");
      if (i != std::string::npos)
        path = path.substr (i + 1);

      sscanf (path.c_str (), "WebMercator_%5d_%5d_%5d_%5d_%5d", &L, &IY0, &IX0, &IY1, &IX1);

      int N = 1;
      float F = 2 * M_PI * a / 256;
      for (int i = 0; i < L; i++)
        N = N * 2;
      program->set1i ("texproj", 1);
      program->set1f ("F", F);
      program->set1i ("N", N);
      program->set1i ("IX0", IX0);
      program->set1i ("IY0", IY0);
      program->set1i ("IX1", IX1);
      program->set1i ("IY1", IY1);
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

glgrib_landscape::~glgrib_landscape ()
{
}





