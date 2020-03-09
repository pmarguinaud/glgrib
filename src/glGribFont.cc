#include "glGribFont.h"
#include "glGribShader.h"
#include "glGribBitmap.h"
#include "glGribResolve.h"

#include <map>


namespace
{
class cache_t : public std::map <std::string,glGrib::FontPtr>
{
};
cache_t cache;
};


glGrib::FontPtr glGrib::newGlgribFontPtr (const glGrib::OptionsFont & opts)
{
  auto it = cache.find (opts.bitmap);
  glGrib::FontPtr font;
  if (it != cache.end ())
    { 
      font = it->second;
    }
  else
    {
      font = std::make_shared<glGrib::Font> ();
      font->setup (opts);
      cache.insert (std::pair<std::string,glGrib::FontPtr> (opts.bitmap, font));
    }
  return font;
}

void glGrib::Font::select () const
{
  glGrib::Program * program = glGrib::Program::load (glGrib::Program::FONT);
  program->use ();
  program->set ("xoff", xoff);
  program->set ("yoff", yoff);
  program->set ("nx", nx);
  program->set ("ny", ny);
  program->set ("aspect", aspect);
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture);
  program->set ("texture", 0);

}

glGrib::Font::~Font ()
{
  if (ready)
    glDeleteTextures (1, &texture);
}

void glGrib::Font::setup (const glGrib::OptionsFont & o)
{
  opts = o;
  unsigned char * rgb = nullptr;
  int w, h;
  std::vector<int> ioff, joff;

  glGrib::Bitmap (glGrib::Resolve (opts.bitmap), &rgb, &w, &h);

  for (int i = 0, p = w * (h - 2); i < w; i++, p += 1)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        xoff.push_back ((float)(i+1)/(float)w);
        ioff.push_back (i+1);
      }

  for (int j = 0, p = 1; j < h; j++, p += w)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        yoff.push_back ((float)(j+1)/(float)h);
        joff.push_back (j+1);
      }

  for (int p = 0; p < w * h; p++)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      { 
        rgb[3*p+0] = 255;
	rgb[3*p+1] = 255; 
	rgb[3*p+2] = 255;
      }

  nx = xoff.size ()-1;
  ny = yoff.size ()-1;


  int rank, ix, iy, pp;

  // Look at nuber of pixels below letter 'i'
  rank = map ('i'); ix = rank % nx; iy = rank / nx;

  pp = 0;
  for (int j = joff[iy]; j < joff[iy+1]; j++)
    for (int i = ioff[ix]; i < ioff[ix+1]; i++)
      {
        int p = j * w + i;
        if (rgb[3*p+0] < 200)
          {
            pp = j - joff[iy];
            goto found_b;
          }
      }
found_b:
  posb = (float)pp / (float)(joff[1] - joff[0]);
  
  // Look at number of pixels above letter 'F'
  rank = map ('F'); ix = rank % nx; iy = rank / nx;

  pp = 0;
  for (int j = joff[iy+1]-1; j > joff[iy]; j--)
   for (int i = ioff[ix]; i < ioff[ix+1]; i++)
      {
        int p = j * w + i;
        if (rgb[3*p+0] < 200)
          {
            pp = joff[iy+1]-1 - j;
            goto found_u;
          }
      }
found_u:
  posu = (float)pp / (float)(joff[1] - joff[0]);

  
  aspect = ((float)w / (float)nx) / ((float)h / (float)ny);

  glGenTextures (1, &texture);
  glBindTexture (GL_TEXTURE_2D, texture); 
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // The third argument has to be GL_RED, but I do not understand why
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);

  delete [] rgb;
  ready = true;
}

