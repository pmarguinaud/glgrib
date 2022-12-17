#include "glGrib/Resolve.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <cstdlib>

#include <iostream>


namespace glGrib
{

std::string glGribPrefix;

std::string Resolve (const std::string & file)
{
  struct stat st;

  // Try current directory
 
  if (stat (file.c_str (), &st) == 0)
    return file;

  // Try user directory

  struct passwd *pw = getpwuid (getuid ());
  const char * home = pw->pw_dir;
  std::string HOME = std::string (home);
  std::string path = HOME + "/.glGribrc/" + file;

  if (stat (path.c_str (), &st) == 0)
    return path;

  const char * GLGRIB_PREFIX = std::getenv ("GLGRIB_PREFIX");
  if (GLGRIB_PREFIX != nullptr)
    {
      const std::string prefix (GLGRIB_PREFIX);
      path = prefix + "/share/glgrib/" + file;
      if (stat (path.c_str (), &st) == 0)
        return path;
    }

  if (glGribPrefix != "")
    {
      path = glGribPrefix + "/share/glgrib/" + file;
      if (stat (path.c_str (), &st) == 0)
        return path;
    }

  // Try install directory
  const int len = 1024;
  int llen;
  char exe[32], prog[len+1];
  sprintf (exe, "/proc/%d/exe", getpid ());
  if ((llen = readlink (exe, prog, len)) > 0)
    {
      prog[llen] = '\0';
      std::string PROG = std::string (prog);
      size_t p = PROG.find_last_of ('/');
      if (p != std::string::npos)
        {
          std::string dir = PROG.substr (0, p + 1);
	  path = dir + "/../share/glgrib/" + file;
	  if (stat (path.c_str (), &st) == 0)
	    return path;
	}
    }
  
  return file;
}

}
