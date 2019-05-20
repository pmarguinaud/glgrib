#ifndef _GLGRIB_X11
#define _GLGRIB_X11

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11 1
#include <GLFW/glfw3native.h>

#include "glgrib_window.h"
#include <string>

extern void glgribX11DrawText (glgrib_window *, const std::string &);

#endif
