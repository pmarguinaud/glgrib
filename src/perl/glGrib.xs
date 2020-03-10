#include "glGribShellInterpreter.h"
#include "glGribResolve.h"
#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"






MODULE = glGrib		PACKAGE = glGrib		


BOOT:
      glGrib::glGribPrefix = GLGRIB_PREFIX "/share";


void
start (...)
CODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();
      std::vector<std::string> args = {"start"};

      for (int i = 0; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);
    }

void
stop ()
CODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();
      std::vector<std::string> args = {"stop"};

      for (int i = 0; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);
    }
    
void 
set (...)
CODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();
      std::vector<std::string> args = {"set"};

      for (int i = 0; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);
    }

void
get (...)
PPCODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();

      std::vector<std::string> args = {"get"};

      for (int i = 0; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);

      const std::vector<std::string> & res = shell.getList ();   
      EXTEND (SP, res.size ());                                  
      for (const auto & s : res)                                 
        PUSHs (sv_2mortal (newSVpv(s.c_str (), 0)));             

    }

void
snapshot (...)
CODE:
    {
      glGrib::ShellInterpreter & shell = glGrib::ShellInterpreter::getInstance ();
      std::vector<std::string> args = {"snapshot"};

      for (int i = 0; i < items; i++)
        args.push_back (std::string ((const char *)SvPV_nolen (ST (i))));

      shell.execute (args);
    }

