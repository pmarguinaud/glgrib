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
  void setup (const Options &);

  class eglDisplay
  {
  public:
    eglDisplay (const std::string &, int, int);
    ~eglDisplay ();
#ifdef USE_EGL
    EGLDisplay display = nullptr;
    EGLContext context = nullptr;
    int fd = -1;
    struct gbm_device * gbm = nullptr;
#endif
  };

  std::shared_ptr<eglDisplay> egl;

};


};
