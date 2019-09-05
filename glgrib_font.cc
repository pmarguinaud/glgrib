#include "glgrib_font.h"
#include "glgrib_shader.h"
#include "glgrib_bitmap.h"
#include "glgrib_resolve.h"

#include <map>


typedef std::map <std::string,glgrib_font_ptr> cache_t;
static cache_t cache;


glgrib_font_ptr new_glgrib_font_ptr (const glgrib_options_font & opts)
{
  auto it = cache.find (opts.bitmap);
  glgrib_font_ptr font;
  if (it != cache.end ())
    { 
      font = it->second;
    }
  else
    {
      font = std::make_shared<glgrib_font> ();
      font->setup (opts);
      cache.insert (std::pair<std::string,glgrib_font_ptr> (opts.bitmap, font));
    }
  return font;
}

void glgrib_font::select () const
{
  glgrib_program * program = glgrib_program_load (glgrib_program::FONT);
  program->use ();
  program->set1fv ("xoff", xoff.data (), xoff.size ());
  program->set1fv ("yoff", yoff.data (), yoff.size ());
  program->set1i ("nx", nx);
  program->set1i ("ny", ny);
  program->set1f ("aspect", aspect);
  glActiveTexture (GL_TEXTURE0); 
  glBindTexture (GL_TEXTURE_2D, texture);
  program->set1i ("texture", 0);

}

glgrib_font::~glgrib_font ()
{
  if (ready)
    glDeleteTextures (1, &texture);
}

void glgrib_font::setup (const glgrib_options_font & o)
{
  opts = o;
  unsigned char * rgb = NULL;
  int w, h;
  std::vector<int> ioff, joff;

  glgrib_bitmap (glgrib_resolve (opts.bitmap), &rgb, &w, &h);

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

  free (rgb);
  ready = true;
}

