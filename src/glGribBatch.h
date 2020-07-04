#pragma once

#ifdef USE_EGL

#include "glGribRender.h"
#include "glGribScene.h"
#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include "glGribShell.h"

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

  virtual class Render * clone () override;

private:
  explicit Batch () {}
  eglDisplay * egl = nullptr;
  void setup (const Options &, EGLContext);
  EGLContext context = nullptr;

};

};
#endif
