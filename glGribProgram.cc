#include "glGribProgram.h"
#include "glGribTrigonometry.h"
#include "glGribOpenGL.h"
#include "glGribShader.h"
#include "glGribResolve.h"

#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <stdexcept>

typedef std::map<std::string,glGrib::Program> name2prog_t;
static name2prog_t name2prog;


static
std::string kind2name (glGrib::Program::kind_t kind)
{
#define KIND(k) do { if (kind == glGrib::Program::k) return #k; } while (0)
  KIND (MONO);
  KIND (SCALAR);
  KIND (LANDSCAPE);
  KIND (VECTOR);
  KIND (CONTOUR);
  KIND (FONT);
  KIND (IMAGE);
  KIND (POINTS);
  KIND (GRID);
  KIND (STREAM);
  KIND (SCALAR_POINTS);
  KIND (FRAME);
  KIND (LAND);
  KIND (ISOFILL1);
  KIND (ISOFILL2);
  KIND (TICKS);
  KIND (FTICKS);
  KIND (TEST);
#undef KIND
  throw std::runtime_error (std::string ("Unknown program kind"));
}

static
std::string slurp (const std::string & file, bool fatal = true)
{
  struct stat st;
  if (stat (file.c_str (), &st) == 0)
    {
      std::string code;
      code.resize (st.st_size + 1);
      FILE * fp = fopen (file.c_str (), "r");
      if (fread (&code[0], st.st_size, 1, fp) != 1)
        throw std::runtime_error (std::string ("Cannot read ") + file);
      fclose (fp);
      code[st.st_size] = 0;
      return code;
    }
  if (fatal)
    throw std::runtime_error (std::string ("Cannot open ") + file);
  return "";
}

void glGrib::Program::read (const std::string & file)
{
  VertexShaderCode   = slurp (glGrib::Resolve (".shaders/" + file + ".vs"));
  FragmentShaderCode = slurp (glGrib::Resolve (".shaders/" + file + ".fs"));
  GeometryShaderCode = slurp (glGrib::Resolve (".shaders/" + file + ".gs"), false);
  name = file;
}

void glGrib::Program::compile ()
{
  if (loaded) 
    return;
  programID = glGrib::LoadShader (name, FragmentShaderCode, VertexShaderCode, GeometryShaderCode);
  matrixID = glGetUniformLocation (programID, "MVP");
  loaded = true;
}

glGrib::Program * glGrib::Program::load (glGrib::Program::kind_t kind)
{
  std::string name = kind2name (kind);
  if (name2prog.find (name) == name2prog.end ())
    {
      glGrib::Program prog;
      name2prog.insert (std::pair<std::string,glGrib::Program> (name, prog));
      name2prog[name].read (name);
      name2prog[name].compile ();
    }
  return &name2prog[kind2name (kind)];
}

glGrib::Program::~Program ()
{
  if (loaded)
    glDeleteProgram (programID);
}


void glGrib::Program::use () const
{
  glUseProgram (programID);
  if (! active)
    {
      glUseProgram (programID);
      active = true;
      for (name2prog_t::iterator it = name2prog.begin (); it != name2prog.end (); it++)
        if (it->second.programID != programID)
          it->second.active = false;
    }
}

void glGrib::Program::set (const glGrib::OptionsLight & light)
{
  int lightid = glGetUniformLocation (programID, "light");

  if (lightid != -1)
    {
      glUniform1i (lightid, light.on);
      if (light.on)
        {
          glm::vec3 lightDir = lonlat2xyz (deg2rad * light.lon, deg2rad * light.lat);
          glUniform3fv (glGetUniformLocation (programID, "lightDir"), 1, &lightDir[0]);
          glUniform1f (glGetUniformLocation (programID, "frac"), light.night);
        }
    }
}



