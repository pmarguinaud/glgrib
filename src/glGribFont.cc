#include "glGribFont.h"
#include "glGribShader.h"
#include "glGribBitmap.h"
#include "glGribResolve.h"
#include "glGribBuffer.h"

#include <map>

namespace glGrib
{

namespace
{
class cache_t : public std::map <std::string,FontPtr>
{
};
cache_t cache;
};

void Font::clearCache ()
{
  cache.clear ();
}


FontPtr getGlGribFontPtr (const OptionsFont & opts)
{
  auto it = cache.find (opts.bitmap.path);
  FontPtr font;
  if (it != cache.end ())
    { 
      font = it->second;
    }
  else
    {
      font = std::make_shared<Font> ();
      font->setup (opts);
      cache.insert (std::pair<std::string,FontPtr> (opts.bitmap.path, font));
    }
  return font;
}

void Font::select () const
{
  Program * program = Program::load ("FONT");
  program->use ();
  program->set ("xoff", xoff);
  program->set ("yoff", yoff);
  program->set ("nx", nx);
  program->set ("ny", ny);
  program->set ("aspect", aspect);
  texture->bind (0);
  program->set ("texture", 0);

}

void Font::setup (const OptionsFont & o)
{
  opts = o;
  BufferPtr<unsigned char> rgb;
  int w, h;
  std::vector<int> ioff, joff;

  Bitmap (Resolve (opts.bitmap.path), rgb, &w, &h);

  for (int i = 0, p = w * (h - 2); i < w; i++, p += 1)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        xoff.push_back (static_cast<float> (i+1)/ static_cast<float> (w));
        ioff.push_back (i+1);
      }

  for (int j = 0, p = 1; j < h; j++, p += w)
    if ((rgb[3*p+0] == 255) && (rgb[3*p+1] == 0) && (rgb[3*p+2] == 0))
      {
        yoff.push_back (static_cast<float> (j+1)/ static_cast<float> (h));
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
  posb = static_cast<float> (pp) / static_cast<float> (joff[1] - joff[0]);
  
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
  posu = static_cast<float> (pp) / static_cast<float> (joff[1] - joff[0]);

  
  aspect = (static_cast<float> (w) / static_cast<float> (nx)) / (static_cast<float> (h) / static_cast<float> (ny));

  texture = OpenGLTexturePtr (w, h, rgb, GL_RED);

  ready = true;
}

}
