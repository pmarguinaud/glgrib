#include "glGrib/ShellRegular.h"
#include "glGrib/WindowSet.h"
#include "glGrib/Options.h"
#include "glGrib/Batch.h"
#include "glGrib/Geometry.h"

#include <iostream>

int main (int argc, const char * argv[])
{
  glGrib::Options opts;

  if (! opts.parse (argc, argv))
    return 1;

  glGrib::glStart (opts.render);

  glGrib::WindowSet * wset = glGrib::WindowSet::create (opts);
  glGrib::ShellRegular & shell = glGrib::ShellRegular::getInstance ();

  if (opts.shell.on)
    {
      shell.setup (opts.shell);
      shell.start (wset);
      if (! shell.isSynchronous ())
        wset->run (&shell);
    }
  else
    {
      wset->run ();
    }

  shell.wait ();

  delete wset;

  glGrib::glStop ();

  return 0;
}


