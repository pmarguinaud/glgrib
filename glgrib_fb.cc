#include <stdio.h>
#include <math.h>
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
  EGLint configAttribs[] = {
          EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
          EGL_BLUE_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_RED_SIZE, 8,
          EGL_DEPTH_SIZE, 8,
          EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
          EGL_NONE
  };    
  
  EGLint pbufferAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE,
  };


  // 1. Initialize EGL
  fb->eglDpy = eglGetDisplay (EGL_DEFAULT_DISPLAY);
  EGLint major, minor;
  eglInitialize (fb->eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig (fb->eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface (fb->eglDpy, eglCfg, pbufferAttribs);

  // 4. Bind the API
  eglBindAPI (EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext (fb->eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);

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
  obj_t Obj;
  prog_t Prog;
  fb_t Fb;

  fb_init (&Fb, width, height);
  
  gl_init ();
  prog_init (&Prog);
  obj_init (&Obj, file);
  view_init (&Prog, &View);
  
  display (&Prog, &Obj, &View);

  glFlush ();
  
  view_free (&View);
  obj_free (&Obj);
  prog_free (&Prog);

  fb_free (&Fb);
  
}

