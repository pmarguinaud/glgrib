#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>

const float halfpi = M_PI / 2;

// Dump RGB to a ppm file

static void screenshot_ppm
(const char *filename, unsigned int width,
 unsigned int height, unsigned char *pixels) 
{
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

// Load/compile shaders utilities

static
GLuint compileShader (const std::string & code, GLuint type)
{
  int len;
  GLint res = GL_FALSE;
  GLuint id = glCreateShader (type);
  const char * str = code.c_str ();

  glShaderSource (id, 1, &str, nullptr);
  glCompileShader (id);

  glGetShaderiv (id, GL_COMPILE_STATUS, &res);
  glGetShaderiv (id, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetShaderInfoLog (id, len, nullptr, &mess[0]);
      throw std::runtime_error (std::string ("Error compiling shader : ") + std::string (mess));
    }

  return id;
}

static
GLuint LoadShader 
  (const std::string & FragmentShaderCode, const std::string & VertexShaderCode)
{
  GLuint VertexShaderID = compileShader (VertexShaderCode, GL_VERTEX_SHADER);
  GLuint FragmentShaderID = compileShader (FragmentShaderCode, GL_FRAGMENT_SHADER);

  // Link the program
  GLuint ProgramID = glCreateProgram ();
  glAttachShader (ProgramID, VertexShaderID);
  glAttachShader (ProgramID, FragmentShaderID);

  glLinkProgram (ProgramID);
  
  // Check the program

  int len;
  GLint res = GL_FALSE;

  glGetProgramiv (ProgramID, GL_LINK_STATUS, &res);
  glGetProgramiv (ProgramID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetProgramInfoLog (ProgramID, len, nullptr, &mess[0]);
      throw std::runtime_error (std::string ("Error linking program : ") + std::string (mess));
    }
  
  glDetachShader (ProgramID, VertexShaderID);
  glDetachShader (ProgramID, FragmentShaderID);
  
  glDeleteShader (VertexShaderID);
  glDeleteShader (FragmentShaderID);

  return ProgramID;
}


// EGL errors

static
bool preEGLError ()
{
  const char * m = nullptr;
  EGLint e = eglGetError (); 
  switch (e)
    {
      case EGL_SUCCESS:
        return true;
      case EGL_NOT_INITIALIZED:
        m = "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
        break;
      case EGL_BAD_ACCESS:
        m = "EGL cannot access a requested resource (for example a context is bound in another thread).";
        break;
      case EGL_BAD_ALLOC:
        m = "EGL failed to allocate resources for the requested operation.";
        break;
      case EGL_BAD_ATTRIBUTE:
        m = "An unrecognized attribute or attribute value was passed in the attribute list.";
        break;
      case EGL_BAD_CONTEXT:
        m = "An EGLContext argument does not name a valid EGL rendering context.";
        break;
      case EGL_BAD_CONFIG:
        m = "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
        break;
      case EGL_BAD_CURRENT_SURFACE:
        m = "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
        break;
      case EGL_BAD_DISPLAY:
        m = "An EGLDisplay argument does not name a valid EGL display connection.";
        break;
      case EGL_BAD_SURFACE:
        m = "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
        break;
      case EGL_BAD_MATCH:
        m = "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface).";
        break;
      case EGL_BAD_PARAMETER:
        m = "One or more argument values are invalid.";
        break;
      case EGL_BAD_NATIVE_PIXMAP:
        m = "A NativePixmapType argument does not refer to a valid native pixmap.";
        break;
      case EGL_BAD_NATIVE_WINDOW:
        m = "A NativeWindowType argument does not refer to a valid native window.";
        break;
      case EGL_CONTEXT_LOST:
        m = "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering. ";
        break;
    }
  if (m != nullptr)
    printf ("%s\n", m);

  exit (1);
  return false;
}

int main (int argc, const char * argv[])
{
  const int width = 800, height = 800;
  const float distance = 6.0f, lon = 0.0f, lat = 0.0f, fov = 20.0f;
  
  // Initialize EGL

  EGLDisplay display = nullptr;
  EGLConfig  config  = nullptr;
  EGLContext context = nullptr;
  struct gbm_device * gbm = nullptr;

  const char * path = "/dev/dri/renderD128";

  int fd = open (path, O_RDWR);

  if (fd < 0)
    throw std::runtime_error (std::string ("Cannot open ") + std::string (path));

  gbm = gbm_create_device (fd);

  if (gbm == nullptr)
    throw std::runtime_error (std::string ("Cannot create gbm object"));

  display = eglGetPlatformDisplay (EGL_PLATFORM_GBM_MESA, gbm, nullptr);
  display || preEGLError ();

  eglInitialize (display, nullptr, nullptr) || preEGLError ();

  const EGLint cfgAttr[] = 
  {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8, EGL_DEPTH_SIZE, 8, EGL_NONE
  };

  EGLint numConfig;

  eglChooseConfig (display, cfgAttr, &config, 1, &numConfig) || preEGLError ();

  eglBindAPI (EGL_OPENGL_API) || preEGLError ();

  const EGLint ctxAttr[] = 
  {
    EGL_CONTEXT_MAJOR_VERSION, 4,
    EGL_CONTEXT_MINOR_VERSION, 4,
    EGL_NONE
  };

  context = eglCreateContext (display, config, EGL_NO_CONTEXT, ctxAttr); 
  context || preEGLError ();

  eglMakeCurrent (display, nullptr, nullptr, context) || preEGLError ();

  // Define object : a single triangle

  std::vector<unsigned int> ind {0, 1, 2};
  std::vector<float> lonlat {0.0f, 0.0f, halfpi, 0.0f, 0.0f, halfpi};

  int numberOfTriangles = ind.size () / 3;

  GLuint vertexbufferID;
  glGenBuffers (1, &vertexbufferID);
  glBindBuffer (GL_ARRAY_BUFFER, vertexbufferID);
  glBufferData (GL_ARRAY_BUFFER, sizeof (float) * lonlat.size (), &lonlat[0], GL_STATIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  GLuint elementbufferID;
  glGenBuffers (1, &elementbufferID);
  glBindBuffer (GL_ARRAY_BUFFER, elementbufferID);
  glBufferData (GL_ARRAY_BUFFER, sizeof (unsigned int) * ind.size (), &ind[0], GL_STATIC_DRAW);

  // View matrix
  
  glm::mat4 MVP;

  glm::vec3 pos
    (distance * glm::cos (glm::radians (float (lon))) * glm::cos (glm::radians (float (lat))), 
     distance * glm::sin (glm::radians (float (lon))) * glm::cos (glm::radians (float (lat))),
     distance *                                         glm::sin (glm::radians (float (lat))));

  float ratio = static_cast<float> (width) / static_cast<float> (height);

  glm::mat4 projection = glm::perspective (glm::radians (fov), ratio, 0.1f, 100.0f);
  glm::mat4 view       = glm::lookAt (pos, glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));
  glm::mat4 model      = glm::mat4 (1.0f);
  MVP = projection * view * model; 

  glViewport (0, 0, width, height);

  glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glCullFace (GL_BACK);
  glFrontFace (GL_CCW);
  glEnable (GL_CULL_FACE);
  glDepthFunc (GL_LESS); 
  glEnable (GL_MULTISAMPLE);

  // Framebuffer

  unsigned int framebuffer;
  unsigned int renderbuffer;
  unsigned int texturebuffer;
  
  glGenFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
  
  glGenTextures (1, &texturebuffer);
  glBindTexture (GL_TEXTURE_2D, texturebuffer);
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, 
                height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, texturebuffer, 0);
  
  glGenRenderbuffers (1, &renderbuffer);
  glBindRenderbuffer (GL_RENDERBUFFER, renderbuffer);
  
  glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, 
                         width, height); 
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                             GL_RENDERBUFFER, renderbuffer); 
  
  if (glCheckFramebufferStatus (GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    throw std::runtime_error (std::string ("Framebuffer is not complete"));
  
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Shader program

  std::string FragmentShaderCode = R"V0G0N(
#version 440 core
out vec4 color;

// Red color

void main()
{
  color.rgb = vec3 (1.0f, 0.0f, 0.0f);
  color.a   = 1.;
}

)V0G0N";

  std::string VertexShaderCode = R"V0G0N(
#version 440 core
layout(location = 0) in vec2 vertexLonLat;

uniform mat4 MVP;

// Get lat/lon coordinates, compute XYZ

void main()
{
  float lon = vertexLonLat.x, lat = vertexLonLat.y;

  float coslon = cos (lon), sinlon = sin (lon);
  float coslat = cos (lat), sinlat = sin (lat);

  vec3 vertexPos = vec3 (coslon * coslat, sinlon * coslat, sinlat);

  gl_Position = MVP * vec4 (vertexPos, 1); 
}

)V0G0N";

  GLuint programID = LoadShader (FragmentShaderCode, VertexShaderCode);
  glUseProgram (programID);
  glUniformMatrix4fv (glGetUniformLocation (programID, "MVP"), 1, GL_FALSE, &MVP[0][0]);

  glDisable (GL_CULL_FACE);

  // Bind buffers

  GLuint VertexArrayID;
  glGenVertexArrays (1, &VertexArrayID);
  glBindVertexArray (VertexArrayID);

  glBindBuffer (GL_ARRAY_BUFFER, vertexbufferID);
  glEnableVertexAttribArray (0); 
  glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, elementbufferID);

  // Draw

  glDrawElements (GL_TRIANGLES, 3 * numberOfTriangles, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray (0);

  glEnable (GL_CULL_FACE);

  // Get picture data and dump to ppm

  std::vector<unsigned char> rgb (width * height * 4);
  glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &rgb[0]);

  for (int i = 0; i < width * height; i++)
    for (int j = 0; j < 3; j++)
      rgb[3*i+j] = rgb[4*i+j];

  screenshot_ppm ("image.ppm", width, height, &rgb[0]);

  // Cleanup

  glDeleteRenderbuffers (1, &renderbuffer); 
  glDeleteTextures (1, &texturebuffer);
  glDeleteFramebuffers (1, &framebuffer);
  glBindFramebuffer (GL_FRAMEBUFFER, 0);

  eglDestroyContext (display, context) || preEGLError ();

  return 0;
}


