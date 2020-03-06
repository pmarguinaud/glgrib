#include "glGribShellPerl.h"
#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

extern "C"
{
  extern void lfiouv_mt64_ ();
}


MODULE = glGrib		PACKAGE = glGrib		

void
hello()
CODE:
    {
    fprintf (stderr, "Hello, world! 0x%llx\n", lfiouv_mt64_);
    glGrib::Shell & shell = glGrib::ShellPerl::getInstance ();
    }
    

