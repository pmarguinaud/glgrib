#include "glgrib_shell.h"

#include <iostream>
#include <exception>

#define C(name,comm,...) glgrib_command (command_##name, comm, #name, __VA_ARGS__)
#define A(...) glgrib_command_arg (__VA_ARGS__)
#define def(name) \
static void command_##name (glgrib_context * ctx, class glgrib_shell * shell, \
                            glgrib_command_arghash & args)

#define F(x) std::stof (args.at (#x).value)
#define I(x) std::stoi (args.at (#x).value)

def (fov)
{
  float dfov = F (dfov);
  ctx->view->fov += dfov;
}

def (rotate)
{
}

def (wireframe)
{
}

def (flat)
{
}

def (close)
{
  shell->close = 1;
}

glgrib_shell Shell 
(
  C (close,     "", A ("", "")),
  C (fov,       "", A ("dfov", "+1.")),
  C (rotate,    "", A ("on",    "1.")),
  C (wireframe, "", A ("on",    "1.")),
  C (flat,      "", A ("on",    "1."))
);

#undef C
#undef A

static std::string next_token (std::string & line)
{
  while (line.length () && line[0] == ' ')
    line = line.substr (1);
  size_t pos = line.find (" ");
  std::string token = line.substr (0, pos);
  if (pos == std::string::npos)
    line = "";
  else
    line = line.substr (pos+1);
  return token;
}

static void help ()
{
  std::cout << "Unknown command" << std::endl;
}

void glgrib_shell::execute (const std::string & _line, glgrib_context * ctx)
{
  std::string line = _line;

  std::string cmd = next_token (line);

  if (cmd == "")
    return;

  if (cmds.count (cmd) == 0)
    return help ();
  
  const glgrib_command & Cmd = cmds.at (cmd);
  const glgrib_command_arglist & Args = Cmd.args;
  glgrib_command_arghash h;

  for (glgrib_command_arglist::const_iterator it = Args.begin ();
       it != Args.end (); it++)
    {
      glgrib_command_arg arg = *it;
      std::string token = next_token (line);
      if (token != "")
        arg.value = token;
      h.insert (std::pair<std::string,glgrib_command_arg>(arg.name, arg));
    }

  try
    {
      Cmd.func (ctx, this, h);
    }
  catch (std::exception & e)
    {
      std::cout << "An error occurred " << std::endl;
      std::cout << e.what() << std::endl;
      std::cout << Cmd.help () << std::endl;
    }
}


