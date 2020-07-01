#pragma once

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

  virtual ~Batch ()
  {
#ifdef USE_EGL
    if (display)
      eglTerminate (display);
    if (gbm)
      gbm_device_destroy (gbm);
    if (fd >= 0)
      ::close (fd);
#endif
  }

  void shouldClose () override
  {
    close ();
  }

  void run (class Shell * = nullptr) override;

  void makeCurrent () override
  {
#ifdef USE_EGL
    eglMakeCurrent (display, nullptr, nullptr, context);
#endif
  }

  void setOptions (const OptionsRender &) override;

  virtual class Render * clone () override;

private:
#ifdef USE_EGL
  EGLDisplay display = nullptr;
  EGLContext context = nullptr;
  int fd = -1;
  struct gbm_device * gbm = nullptr;
#endif
};


};
