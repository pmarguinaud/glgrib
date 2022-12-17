#include "glGrib/ShellInterpreter.h"
#include "glGrib/Resolve.h"
#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"






MODULE = glGrib		PACKAGE = glGrib		


BOOT:
      glGrib::glGribPrefix = GLGRIB_PREFIX;


void
_method_ (...)
PPCODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();
      std::vector<std::string> args;

      for (int i = 1; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);

      if (SvIV (ST (0)))
        {
          const std::vector<std::string> & res = shell.getList ();   
          EXTEND (SP, res.size ());                                  
          for (const auto & s : res)                                 
            PUSHs (sv_2mortal (newSVpv(s.c_str (), 0)));             
        }

    }

