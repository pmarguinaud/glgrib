#include <omp.h>

#include "glGribOptions.h"
#include "glGribOpenGL.h"
#include "glGribBatch.h"

#include <iostream>

#include <assert.h>
#include <fcntl.h>
#include <gbm.h>
#include <unistd.h>
#include <string.h>

namespace
{

void screenshot_ppm
(const char *filename, unsigned int width,
 unsigned int height, unsigned char *pixels)
{
  const size_t format_nchannels = 3;
  FILE * fp = fopen (filename, "w");
  fprintf (fp, "P3\n%d %d\n%d\n", width, height, 255);
  for (int i = 0; i < static_cast<int> (height); i++) 
    {
      for (int j = 0; j < static_cast<int> (width); j++) 
        {
          int cur = format_nchannels * ((height - i - 1) * width + j);
          fprintf (fp, "%3d %3d %3d ", (pixels)[cur], (pixels)[cur + 1], (pixels)[cur + 2]);
        }
      fprintf(fp, "\n");
    }
  fclose (fp);
}

}

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if (! opts.parse (argc, argv))
    return 1;

  glGrib::Batch batch (opts);

  const int width = opts.window.width, height = opts.window.height;

  // Create framebuffer for rendering

  GLuint fbo;
  GLuint rbo_color;
  GLuint rbo_depth;

  glGenFramebuffers (1, &fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, fbo);

  glGenRenderbuffers (1, &rbo_color);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo_color);
  /* Storage must be one of: */
  /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
  glRenderbufferStorage (GL_RENDERBUFFER, GL_RGB565, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_RENDERBUFFER, rbo_color);

  glGenRenderbuffers (1, &rbo_depth);
  glBindRenderbuffer (GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                             GL_RENDERBUFFER, rbo_depth);


  glReadBuffer (GL_COLOR_ATTACHMENT0);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glGrib::glInit ();

  glViewport (0, 0, width, height);

  // Render

  batch.getScene ().render ();

  glFlush ();


  // Write image to disk
  unsigned char * pixels = new unsigned char[3 * width * height];

  std::fill (pixels, pixels + 3 * width * height, 0);

  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  screenshot_ppm ("toto.ppm", width, height, pixels);
  free (pixels);

  // Cleanup
  glDeleteFramebuffers (1, &fbo);
  glDeleteRenderbuffers (1, &rbo_color);
  glDeleteRenderbuffers (1, &rbo_depth);

  return 0;
}


