#include "glGribRender.h"
#include "glGribSnapshot.h"
#include "glGribClear.h"

void APIENTRY debugCallback 
  (unsigned int source, unsigned int type, GLuint id, unsigned int severity, 
   int length, const char * message, const void * data)
{
  glGrib::Render * render = (glGrib::Render*)data;
  render->debug (source, type, id, severity, length, message);
}

glGrib::Render::Render (const Options & opts)
{
}

void glGrib::Render::setupDebug ()
{
  if (opts.debug.on || opts.info.on)
   {
#define pp(x) \
     printf ("%-40s : %s\n", #x, glGetString (x));
     pp (GL_VENDOR);
     pp (GL_RENDERER);
     pp (GL_VERSION); 
     pp (GL_SHADING_LANGUAGE_VERSION);
     pp (GL_EXTENSIONS);
#undef pp
   }
  if (opts.debug.on)
   {
     GLint flags; 
     glGetIntegerv (GL_CONTEXT_FLAGS, &flags);
     if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
       {
         glEnable (GL_DEBUG_OUTPUT);
         glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); 
         glDebugMessageCallback (debugCallback, this);
         glDebugMessageControl (GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
       }
   }

}

void glGrib::Render::snapshot (const std::string & format)
{
  glGrib::snapshot (*this, format);
}

void glGrib::Render::framebuffer (const std::string & format)
{
  glGrib::framebuffer (*this, format);
}

void glGrib::Render::reSize (int w, int h)
{
  opts.width = w;
  opts.height = h;
  makeCurrent ();
  glViewport (0, 0, opts.width, opts.height);
  scene.setViewport (opts.width, opts.height);
}

void glGrib::Render::clear ()
{
  cleared = false;
  scene.clear ();
}

namespace
{


#define GLMESS(x) case GL_DEBUG_SOURCE_##x: return #x
const char * debugSource (unsigned int source)
{
  switch (source)
    {
      GLMESS (API); GLMESS (WINDOW_SYSTEM); GLMESS (SHADER_COMPILER);
      GLMESS (THIRD_PARTY); GLMESS (APPLICATION); GLMESS (OTHER);
    }
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_TYPE_##x: return #x
const char * debugType (unsigned int type)
{
  switch (type)
    {
      GLMESS (ERROR); GLMESS (DEPRECATED_BEHAVIOR); GLMESS (UNDEFINED_BEHAVIOR);
      GLMESS (PORTABILITY); GLMESS (PERFORMANCE); GLMESS (MARKER);
      GLMESS (PUSH_GROUP); GLMESS (POP_GROUP); GLMESS (OTHER);
    } 
  return "UNKNOWN";
}
#undef GLMESS

#define GLMESS(x) case GL_DEBUG_SEVERITY_##x: return #x
const char * debugSeverity (unsigned int severity)
{
  switch (severity)
    {
      GLMESS (HIGH); GLMESS (MEDIUM);
      GLMESS (LOW); GLMESS (NOTIFICATION);
    } 
  return "UNKNOWN";
}
#undef GLMESS

}

void glGrib::Render::debug (unsigned int source, unsigned int type, GLuint id, 
		            unsigned int severity, int length, const char * message)
{
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) 
    return; 
  printf ("%-20s | %-20s | %-30s | %10d | %s\n", debugSource (source), 
          debugSeverity (severity), debugType (type), id, message);
}

