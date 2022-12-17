#pragma once

#ifdef GLGRIB_USE_EGL

#include "glGrib/Render.h"
#include "glGrib/Scene.h"
#include "glGrib/OpenGL.h"
#include "glGrib/Options.h"
#include "glGrib/Shell.h"

#include <unistd.h>

namespace glGrib
{

class Shell;

class Batch : public Render
{
public:

  explicit Batch (const Options &);

  virtual ~Batch ();

  void shouldClose () override
  {
    close ();
  }

  void run (class Shell * = nullptr) override;

  void makeCurrent () override;

  void setOptions (const OptionsRender &) override;

  virtual class Render * clone (bool = true) override;

private:
  explicit Batch () {}
  eglDisplay * egl = nullptr;
  void setup (const Options &, EGLContext);
  EGLContext context = nullptr;

};

};
#endif
