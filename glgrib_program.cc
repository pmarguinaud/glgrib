#include "glgrib_program.h"
#include "glgrib_trigonometry.h"
#include "glgrib_opengl.h"
#include "glgrib_shader.h"
#include "glgrib_resolve.h"

#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <stdexcept>

typedef std::map<std::string,glgrib_program> name2prog_t;
static name2prog_t name2prog;


static
std::string kind2name (glgrib_program::kind kind)
{
#define KIND(k) do { if (kind == glgrib_program::k) return #k; } while (0)
  KIND (RGBA);
  KIND (RGB);
  KIND (MONO);
  KIND (RGB_FLAT);
  KIND (SCALAR);
  KIND (FLAT_TEX);
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

void glgrib_program::read (const std::string & file)
{
  VertexShaderCode   = slurp (glgrib_resolve (".shaders/" + file + ".vs"));
  FragmentShaderCode = slurp (glgrib_resolve (".shaders/" + file + ".fs"));
  GeometryShaderCode = slurp (glgrib_resolve (".shaders/" + file + ".gs"), false);
}

void glgrib_program::compile ()
{
  if (loaded) 
    return;
  programID = glgrib_load_shader (FragmentShaderCode, VertexShaderCode, GeometryShaderCode);
  matrixID = glGetUniformLocation (programID, "MVP");
  loaded = true;
}

glgrib_program * glgrib_program::load (glgrib_program::kind kind)
{
  std::string name = kind2name (kind);
  if (name2prog.find (name) == name2prog.end ())
    {
      glgrib_program prog;
      name2prog.insert (std::pair<std::string,glgrib_program> (name, prog));
      name2prog[name].read (name);
      name2prog[name].compile ();
    }
  return &name2prog[kind2name (kind)];
}

glgrib_program::~glgrib_program ()
{
  if (loaded)
    glDeleteProgram (programID);
}


void glgrib_program::use () const
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

void glgrib_program::set (const glgrib_options_light & light)
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



