#include <string.h>
#include <stdio.h>
#include <EGL/egl.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glut.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <png.h>


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

static void screenshot_png(const char *filename, unsigned int width, unsigned int height,
        GLubyte *pixels, png_byte *png_bytes, png_byte **png_rows) {
    size_t i, nvals;
    const size_t format_nchannels = 4;
    FILE *f = fopen(filename, "wb");
    nvals = format_nchannels * width * height;
    for (i = 0; i < nvals; i++)
        png_bytes[i] = pixels[i];
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();
    png_infop info = png_create_info_struct(png);
    if (!info) abort();
    if (setjmp(png_jmpbuf(png))) abort();
    png_init_io(png, f);
    png_set_IHDR(
        png,
        info,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    png_write_image(png, png_rows);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
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

  const EGLint config_attr[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
  };    

  eglChooseConfig(eglDpy, config_attr, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  const EGLint surf_attr[] = {
      EGL_WIDTH, 100,
      EGL_HEIGHT, 100,
      EGL_NONE,
  };

  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, surf_attr);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current

  const EGLint ctx_attr[] = 
  {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
  };

  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, ctx_attr);
//EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  // from now on use your OpenGL context

  GLuint ProgramID;
{
  int len;
  GLint res = GL_FALSE;
  GLuint VertexShaderID = glCreateShader (GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader (GL_FRAGMENT_SHADER);

  const char * FragmentShaderCode =
"#version 330 core\n"
"\n"
"in vec4 fragmentColor;\n"
"\n"
"out vec4 color;\n"
"\n"
"void main(){\n"
"\n"
"        color.r = fragmentColor.r;\n"
"        color.g = fragmentColor.g;\n"
"        color.b = fragmentColor.b;\n"
"        color.a = fragmentColor.a;\n"
"}\n";

  const char * VertexShaderCode = 
"#version 330 core\n"
"\n"
"layout(location = 0) in vec3 vertexPosition_modelspace;\n"
"layout(location = 1) in vec4 vertexColor;\n"
"\n"
"out vec4 fragmentColor;\n"
"uniform mat4 MVP;\n"
"\n"
"void main(){\n"
"\n"
"        gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
"\n"
"        fragmentColor.r = vertexColor.r;\n"
"        fragmentColor.g = vertexColor.g;\n"
"        fragmentColor.b = vertexColor.b;\n"
"        fragmentColor.a = vertexColor.a;\n"
"}\n";


  // Compile Vertex Shader
  glShaderSource (VertexShaderID, 1, &VertexShaderCode , NULL);
  glCompileShader (VertexShaderID);

  glGetShaderiv (VertexShaderID, GL_COMPILE_STATUS, &res);
  glGetShaderiv (VertexShaderID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (VertexShaderID, len, NULL, &mess[0]);
      printf("%s\n", mess);
      abort ();
    }


  // Compile Fragment Shader
  glShaderSource (FragmentShaderID, 1, &FragmentShaderCode , NULL);
  glCompileShader (FragmentShaderID);

  glGetShaderiv (FragmentShaderID, GL_COMPILE_STATUS, &res);
  glGetShaderiv (FragmentShaderID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (FragmentShaderID, len, NULL, &mess[0]);
      printf("%s\n", mess);
      abort ();
    }

  // Link the program
  ProgramID = glCreateProgram ();
  glAttachShader (ProgramID, VertexShaderID);
  glAttachShader (ProgramID, FragmentShaderID);
  glLinkProgram (ProgramID);
  
  // Check the program
  glGetProgramiv (ProgramID, GL_LINK_STATUS, &res);
  glGetProgramiv (ProgramID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetProgramInfoLog (ProgramID, len, NULL, &mess[0]);
      printf("%s\n", mess);
      abort ();
    }
  
  
  glDetachShader (ProgramID, VertexShaderID);
  glDetachShader (ProgramID, FragmentShaderID);
  
  glDeleteShader (VertexShaderID);
  glDeleteShader (FragmentShaderID);
  
}

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


GLuint VertexArrayID;
GLuint vertexbuffer, colorbuffer, elementbuffer;
{
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);
  
  float * xyz = (float *)malloc (3 * 3 * sizeof (float));
  float * col = (float *)malloc (3 * 4 * sizeof (float));
  unsigned int * ind = (unsigned int *)malloc (1 * 3);

  xyz[0*3+0] = +0.0; xyz[0*3+1] = +2.0; xyz[0*3+2] = +0.0;
  xyz[1*3+0] = +0.0; xyz[1*3+1] = +0.0; xyz[1*3+2] = +2.0;
  xyz[2*3+0] = +0.0; xyz[2*3+1] = -2.0; xyz[2*3+2] = +0.0;

  for (int i = 0; i < 3; i++)
  for (int j = 0; j < 4; j++)
    col[4*i+j] = 1.0;

  ind[0] = 0;
  ind[1] = 1;
  ind[2] = 2;

  
  glGenBuffers (1, &vertexbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData (GL_ARRAY_BUFFER, 3 * 3 * sizeof (float), xyz, GL_STATIC_DRAW);
  

  glGenBuffers (1, &colorbuffer);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glBufferData (GL_ARRAY_BUFFER, 4 * 3 * sizeof (float), col, GL_STATIC_DRAW);
  
  glGenBuffers (1, &elementbuffer);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, 3 * 1 * sizeof (unsigned int), 
		ind , GL_STATIC_DRAW);

  free (ind);
  free (xyz);
  free (col);
}

  glReadBuffer(GL_COLOR_ATTACHMENT0);

  glm::mat4 Projection = glm::perspective (glm::radians (50.0f), 1.0f / 1.0f, 0.1f, 100.0f);
  glm::mat4 View       = glm::lookAt (glm::vec3 (3,0,0), glm::vec3 (0,0,0), glm::vec3 (0,0,1));
  glm::mat4 Model      = glm::mat4 (1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (ProgramID);
  
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  
  glEnableVertexAttribArray (1);
  glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
  glVertexAttribPointer (1, 4, GL_FLOAT, GL_TRUE, 4 * sizeof (float), NULL);
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glDrawElements (GL_TRIANGLES, 3 * 1, GL_UNSIGNED_INT, NULL);
  
  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);



  glFlush();

{
  GLubyte * pixels = (GLubyte *)malloc(3 * sizeof (GLubyte) * width * height);
  memset (pixels, 0, 3 * sizeof (GLubyte) * width * height);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  screenshot_ppm ("toto.ppm", width, height, pixels);
  free (pixels);
}
{
  GLubyte * pixels = (GLubyte *)malloc(4 * sizeof (GLubyte) * width * height);
  png_byte *png_bytes = (png_byte *)malloc (4 * width *height * sizeof(png_byte));
  png_byte **png_rows = (png_byte **)malloc(height * sizeof(png_byte*));
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  for (int i = 0; i < height; i++)
      png_rows[height - i - 1] = &png_bytes[i * width * 4];
  screenshot_png("toto.png", width, height, pixels, png_bytes, png_rows);
  free (png_bytes);
  free (png_rows);
  free (pixels);
}

  glDeleteFramebuffers(1, &fbo);
  glDeleteRenderbuffers(1, &rbo_color);
  glDeleteRenderbuffers(1, &rbo_depth);
  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}
