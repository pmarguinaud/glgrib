#include "glGribProgram.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"
#include "glGribShader.h"
#include "glGribResolve.h"

#include <string>
#include <cmath>
#include <glm/glm.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <stdexcept>
#include <fstream>

namespace glGrib
{

namespace
{
typedef std::map<std::string,Program> name2prog_t;
name2prog_t name2prog;

std::string slurp (const std::string & file, bool fatal = true)
{

  std::ifstream fh (file, std::ios::in | std::ios::binary | std::ios::ate);

  if (fh.is_open ())
    {
      size_t size = fh.tellg ();
      std::string code (size + 1, '\0');
      fh.seekg (0, std::ios::beg);
      fh.read (&code[0], size);
      fh.close ();
      return code;
    }
  else if (fatal)
    throw std::runtime_error (std::string ("Cannot open ") + file);

  return "";
}

}

void Program::clearCache ()
{
  name2prog.clear ();
}

void Program::read (const std::string & file)
{
  VertexShaderCode   = slurp (Resolve ("/shaders/" + file + ".vs"));
  FragmentShaderCode = slurp (Resolve ("/shaders/" + file + ".fs"));
  GeometryShaderCode = slurp (Resolve ("/shaders/" + file + ".gs"), false);
  name = file;
}

void Program::compile ()
{
  if (loaded) 
    return;
  programID = LoadShader (name, FragmentShaderCode, VertexShaderCode, GeometryShaderCode);
  loaded = true;
}

Program * Program::load (const std::string & name)
{
  if (name2prog.find (name) == name2prog.end ())
    {
      Program prog;
      name2prog.insert (std::pair<std::string,Program> (name, prog));
      name2prog[name].read (name);
      name2prog[name].compile ();
    }
  return &name2prog[name];
}

Program::~Program ()
{
  if (loaded)
    glDeleteProgram (programID);
}


void Program::use () const
{
  glUseProgram (programID);
  if (! active)
    {
      glUseProgram (programID);
      active = true;
      for (name2prog_t::iterator it = name2prog.begin (); it != name2prog.end (); ++it)
        if (it->second.programID != programID)
          it->second.active = false;
    }
}

void Program::set (const OptionsLight & light)
{
  int lightid = getUniformLocation ("light");

  if (lightid != -1)
    {
      glUniform1i (lightid, light.on);
      if (light.on)
        {
          glm::vec3 lightDir = lonlat2xyz (deg2rad * light.lon, deg2rad * light.lat);
          glUniform3fv (getUniformLocation ("lightDir"), 1, &lightDir[0]);
          glUniform1f (getUniformLocation ("frac"), light.night);
        }
    }
}

}

