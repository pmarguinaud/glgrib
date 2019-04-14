#ifndef _GLGRIB_OPENGL_H
#define _GLGRIB_OPENGL_H

#ifdef USE_GLE
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glut.h>
#include <GL/glext.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

void gl_init ();

#endif
