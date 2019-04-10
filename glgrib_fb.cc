#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glut.h>
#include <GL/glext.h>
#include "glgrib_render.h"
#include "glgrib_fb.h"

using namespace glm;

typedef struct fb_t
{
  GLuint fbo;
  GLuint rbo_color;
  GLuint rbo_depth;
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

  // Use OpenGL: create framebuffer

  glGenFramebuffers (1, &fb->fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, fb->fbo);

  /* Color renderbuffer. */
  glGenRenderbuffers (1, &fb->rbo_color);
  glBindRenderbuffer (GL_RENDERBUFFER, fb->rbo_color);
  /* Storage must be one of: */
  /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
  glRenderbufferStorage (GL_RENDERBUFFER, GL_RGB565, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                             GL_RENDERBUFFER, fb->rbo_color);

  /* Depth renderbuffer. */
  glGenRenderbuffers (1, &fb->rbo_depth);
  glBindRenderbuffer (GL_RENDERBUFFER, fb->rbo_depth);
  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer (GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                             GL_RENDERBUFFER, fb->rbo_depth);

  glReadBuffer (GL_COLOR_ATTACHMENT0);
}

static
void fb_free (fb_t * fb)
{
  glDeleteFramebuffers (1, &fb->fbo);
  glDeleteRenderbuffers (1, &fb->rbo_color);
  glDeleteRenderbuffers (1, &fb->rbo_depth);
  eglTerminate (fb->eglDpy);
}

void fb_display (const char * file, int width, int height)
{
  scene_t Scene;
  world_t World;
  cube_t Cube;
  prog_t Prog;
  view_t View;
  fb_t Fb;

  fb_init (&Fb, width, height);
  
  gl_init ();
  Prog.init ();
  World.init (file);
  Cube.init ();
  View.init (&Prog);

  Scene.objlist.push_back (&World);
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

