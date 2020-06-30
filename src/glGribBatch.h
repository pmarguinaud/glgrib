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
  }

  void run (class Shell * = nullptr) override;

  void makeCurrent () override
  {
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
#endif
};


};
