#include <string.h>
#include <stdio.h>
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glut.h>
#include <GL/glext.h>

static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
};    

static const EGLint pbufferAttribs[] = {
      EGL_WIDTH, 100,
      EGL_HEIGHT, 100,
      EGL_NONE,
};

static void screenshot_ppm(const char *filename, unsigned int width,
        unsigned int height, GLubyte *pixels) {
    size_t i, j, cur;
    const size_t format_nchannels = 3;
    FILE *f = fopen(filename, "w");
    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur = format_nchannels * ((height - i - 1) * width + j);
            fprintf(f, "%3d %3d %3d ", (pixels)[cur], (pixels)[cur + 1], (pixels)[cur + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

int main(int argc, char *argv[])
{
  const int width = 100, height = 100;
  GLuint fbo;
  GLuint rbo_color;
  GLuint rbo_depth;

  // 1. Initialize EGL
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;

  eglInitialize(eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, 
                                               pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, 
                                       NULL);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  // from now on use your OpenGL context
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);


  /* Color renderbuffer. */
  glGenRenderbuffers(1, &rbo_color);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_color);
  /* Storage must be one of: */
  /* GL_RGBA4, GL_RGB565, GL_RGB5_A1, GL_DEPTH_COMPONENT16, GL_STENCIL_INDEX8. */
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, width, height);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                            GL_RENDERBUFFER, rbo_color);

  /* Depth renderbuffer. */
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                            GL_RENDERBUFFER, rbo_depth);

  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glRotatef(0.0f, 0.0f, 0.0f, -1.0f);
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f( 0.0f,  0.5f, 0.0f);
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, 0.0f);
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f( 0.5f, -0.5f, 0.0f);
  glEnd();

  glFlush();
  GLubyte * pixels = (GLubyte *)malloc(3 * sizeof (GLubyte) * width * height);
  memset (pixels, 0, 3 * sizeof (GLubyte) * width * height);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  free (pixels);

  int c[3] = {0, 0, 0};
  for (int i = 0; i < width * height; i++)
    for (int j = 0; j < 3; j++)
      if (pixels[3*i+j] != 0)
        c[j]++;

  for (int j = 0; j < 3; j++)
    printf (" c[%d] = %d\n", j, c[j]);
    
  screenshot_ppm ("toto.ppm", width, height, pixels);


  glDeleteFramebuffers(1, &fbo);
  glDeleteRenderbuffers(1, &rbo_color);
  glDeleteRenderbuffers(1, &rbo_depth);
  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}
