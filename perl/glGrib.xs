#include "glGribShellPerl.h"
#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"


MODULE = glGrib		PACKAGE = glGrib		


void
hello()
CODE:
    {
    glGrib::ShellPerl & shell = glGrib::ShellPerl::getInstance ();
    shell.start (nullptr);
    fprintf(stderr,"coucou\n");
    sleep (10);
    }
    

