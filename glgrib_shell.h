#ifndef _GLGRIB_SHELL
#define _GLGRIB_SHELL

#include "glgrib_window.h"

#include <string>
#include <map>
#include <list>

class glgrib_command_arg
{
private:
  glgrib_command_arg () {}
public:
  glgrib_command_arg (std::string n, std::string d = "", std::string c = "") 
    : name (n), value (d), comment (c) {}
  std::string name, value = "", comment = "";
};

typedef std::list <glgrib_command_arg> glgrib_command_arglist;
typedef std::map <std::string,glgrib_command_arg> glgrib_command_arghash;
typedef void (*glgrib_command_func) (class glgrib_window * ctx, class glgrib_shell *, glgrib_command_arghash &);

class glgrib_command
{
public:
  glgrib_command (glgrib_command_func f, std::string n, std::string c)
  {
    func = f;
    name = n;
    comment = c;
  }
  template <typename T, typename... Types> 
  glgrib_command (glgrib_command_func f, std::string n, std::string c, 
                  T arg, Types... vars)
  {
    glgrib_command cmd = glgrib_command (f, n, c, vars...);
    func = cmd.func;
    name = cmd.name;
    args = cmd.args;
    comment = cmd.comment;
    args.push_back (arg);
  }
  glgrib_command_func func = (glgrib_command_func)0;
  std::string name, comment = "";
  glgrib_command_arglist args;
  std::string help () const { return comment; }
};

class glgrib_shell
{
public:
  glgrib_shell () { }
  template <typename T, typename... Types> 
  glgrib_shell (T cmd, Types... vars)
  {
    glgrib_shell shell = glgrib_shell (vars...);
    cmds = shell.cmds;
    cmds.insert (std::pair<std::string,glgrib_command>(cmd.name, cmd));
  }
  void execute (const std::string &, class glgrib_window *);
  int close = 0;
  std::map <std::string,glgrib_command> cmds;
  bool closed () { return close; }
  void run (class glgrib_window_set *);
  int windowid = 0;
private:
  glgrib_window_set * wset = NULL;
};

extern glgrib_shell Shell;

#endif
