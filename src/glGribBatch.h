#pragma once

#include "glGribRender.h"
#include "glGribScene.h"
#include "glGribOpenGL.h"
#include "glGribOptions.h"
#include "glGribShell.h"

namespace glGrib
{

class Shell;

class Batch : public Render
{
public:

  explicit Batch (const Options &);

  virtual ~Batch ()
  {
#ifdef USE_EGL
    eglTerminate (display);
#endif
  }

  void shouldClose () override
  {
    close ();
  }

  void run (class Shell * = nullptr) override;

  void makeCurrent () override
  {
    eglMakeCurrent (display, nullptr, nullptr, context);
  }

  void setOptions (const OptionsWindow &) override
  {
  }

  virtual class Render * clone () override
  {
    return nullptr;
  }

private:
#ifdef USE_EGL
  EGLDisplay display = nullptr;
  EGLContext context = nullptr;
#endif
};


};
