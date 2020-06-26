#include "glGribShellRegular.h"
#include "glGribWindowSet.h"
#include "glGribOptions.h"
#include "glGribBatch.h"

#include <iostream>


namespace
{

void runWindow (const glGrib::Options & opts)
{
  glGrib::WindowSet * wset = glGrib::WindowSet::create (opts);
  glGrib::Shell & shell = glGrib::ShellRegular::getInstance ();

  if (opts.shell.on)
    {
      shell.setup (opts.shell);
      shell.start (wset);
      wset->run (&shell);
    }
  else
    {
      wset->run ();
    }

  shell.wait ();

  delete wset;
}

void runBatch (const glGrib::Options & opts)
{
  glGrib::Batch batch (opts);
  batch.run ();
}

}

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if (! opts.parse (argc, argv))
    return 1;

  glGrib::glStart ();

  bool use_glfw = false, use_egl = false;

#ifdef USE_GLFW
  use_glfw = true;
#endif
#ifdef USE_EGL
  use_egl = true;
#endif
 
  if (use_glfw)
    runWindow (opts);  
  if (use_egl)
    runBatch (opts);

  glGrib::glStop ();

  return 0;
}


