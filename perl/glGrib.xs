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


    for (const auto & s : shell.getListStr ())
      {
        std::cerr << s << std::endl;
      }

    fprintf(stderr,"coucou\n");
    sleep (10);
    }
    

