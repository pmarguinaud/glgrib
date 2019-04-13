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


int main(int argc, char *argv[])
{

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


  EGLint attrib_list[] = 
  {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
  };

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, attrib_list);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  // from now on use your OpenGL context

  int len;
  GLint res = GL_FALSE;
  GLuint VertexShaderID = glCreateShader (GL_VERTEX_SHADER);

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
    }

  // Link the program
  GLuint ProgramID = glCreateProgram ();
  glAttachShader (ProgramID, VertexShaderID);
  glLinkProgram (ProgramID);
  
  // Check the program
  glGetProgramiv (ProgramID, GL_LINK_STATUS, &res);
  glGetProgramiv (ProgramID, GL_INFO_LOG_LENGTH, &len);
  if (len > 0)
    {
      char mess[len+1];
      glGetProgramInfoLog (ProgramID, len, NULL, &mess[0]);
      printf("%s\n", mess);
    }
  
  
  glDetachShader (ProgramID, VertexShaderID);
  
  glDeleteShader (VertexShaderID);
  

  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}
