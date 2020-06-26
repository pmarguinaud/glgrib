#pragma once

#include "glGribScene.h"
#include "glGribOpenGL.h"
#include "glGribOptions.h"

namespace glGrib
{

class Batch
{
public:

  explicit Batch (const Options &);

  virtual ~Batch ()
  {
    eglTerminate (display);
  }

  int & getSnapshotCnt ()
  {
    return snapshot_cnt;
  }
  
  Scene & getScene ()
  {
    return scene;
  }

  const OptionsWindow & getOptions () const
  {
    return opts;
  }

  void run ();

private:
  OptionsWindow opts;
  int snapshot_cnt = 0;
  Scene scene;
  EGLDisplay display = nullptr;
};


};
