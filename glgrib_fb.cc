#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include "glgrib_fb.h"
#include "glgrib_png.h"
#include "glgrib_world.h"
#include "glgrib_scene.h"
#include "glgrib_cube.h"
#include "glgrib_cube1.h"
#include "glgrib_grid.h"
#include "glgrib_view.h"
#include "glgrib_program.h"
#include "glgrib_coastlines.h"

typedef struct fb_t
{
  GLuint framebuffer;
  GLuint textureColorbuffer;
  GLuint rbo;
  EGLDisplay eglDpy;
} fb_t;



static
void fb_init (fb_t * fb, int width, int height)
{
  const EGLint config_attr[] = 
  {
          EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
          EGL_BLUE_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_RED_SIZE, 8,
          EGL_DEPTH_SIZE, 8,
          EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
          EGL_NONE
  };    
  
  const EGLint pbuffer_attr[] = 
  {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE,
  };

  const EGLint ctx_attr[] = 
  {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
  };


  // 1. Initialize EGL
  fb->eglDpy = eglGetDisplay (EGL_DEFAULT_DISPLAY);
  EGLint major, minor;
  eglInitialize (fb->eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig (fb->eglDpy, config_attr, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface (fb->eglDpy, eglCfg, pbuffer_attr);

  // 4. Bind the API
  eglBindAPI (EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext (fb->eglDpy, eglCfg, EGL_NO_CONTEXT, ctx_attr);

  eglMakeCurrent (fb->eglDpy, eglSurf, eglSurf, eglCtx);

  //

  glGenFramebuffers (1, &fb->framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, fb->framebuffer);
 

  glGenTextures (1, &fb->textureColorbuffer);
  glBindTexture (GL_TEXTURE_2D, fb->textureColorbuffer);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->textureColorbuffer, 0);

  glGenRenderbuffers (1, &fb->rbo);
  glBindRenderbuffer (GL_RENDERBUFFER, fb->rbo);

  // use a single renderbuffer object for both a depth AND stencil buffer.
  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height); 
  // now actually attach it
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb->rbo); 

#define CASE(st) case st: printf (#st "\n"); break
  GLenum ret = glCheckFramebufferStatus (GL_FRAMEBUFFER);
  switch (ret)
    {
       CASE (GL_FRAMEBUFFER_COMPLETE);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
       CASE (GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );
       CASE (GL_FRAMEBUFFER_UNDEFINED);
       CASE (GL_FRAMEBUFFER_UNSUPPORTED);
       CASE (GL_INVALID_ENUM); 
       CASE (GL_INVALID_OPERATION);
       default:
         printf ("glCheckFramebufferStatus returned unknown status %d\n", ret);
         break;
    }
#undef CASE


}

static
void fb_free (fb_t * fb)
{
  glDeleteFramebuffers (1, &fb->framebuffer);
  glDeleteTextures (1, &fb->textureColorbuffer);
  glDeleteRenderbuffers (1, &fb->rbo);
  eglTerminate (fb->eglDpy);
}

void fb_display (const char * file, int width, int height)
{
  glgrib_scene Scene;
  glgrib_world World;
  glgrib_cube1 Cube;
  glgrib_program Prog;
  glgrib_view View;
  fb_t Fb;

  fb_init (&Fb, width, height);
  
  gl_init ();
  Prog.init (0);
//World.init (file);
  Cube.init ();
  View.init (&Prog);

//Scene.objlist.push_back (&World);
  Scene.objlist.push_back (&Cube);
  Scene.view = &View;
  Scene.prog = &Prog;
  
  Scene.display ();

  glFlush ();

{
  GLubyte * pixels = (GLubyte *)malloc (4 * sizeof (GLubyte) * width * height);
  memset (pixels, 0, 4 * sizeof (GLubyte) * width * height);
  glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  int count = 0;
  for (int i = 0; i < 4 * width * height; i++)
    if (pixels[i] > 0)
      count++;
  printf ("count = %d\n", count);
}
  
  
  fb_free (&Fb);
  
}

