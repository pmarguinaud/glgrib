#include "glGribResolve.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#include <iostream>


std::string glGrib::Resolve (const std::string & file)
{
  // Try user directory

  struct passwd *pw = getpwuid (getuid ());
  const char * home = pw->pw_dir;
  std::string HOME = std::string (home);
  std::string path = HOME + "/.glgribrc/" + file;
  struct stat st;
  if (stat (path.c_str (), &st) == 0)
    return path;


  // Try install directory
  int len = 256;
  char exe[32], prog[len+1];
  sprintf (exe, "/proc/%d/exe", getpid ());
  if (readlink (exe, prog, len) > 0)
    {
      std::string PROG = std::string (prog);
      int p = PROG.find_last_of ('/');
      if (p != std::string::npos)
        {
          std::string dir = PROG.substr (0, p + 1);
	  std::string path = dir + file;
	  if (stat (path.c_str (), &st) == 0)
	    return path;
	}
    }
  
  return file;
}

