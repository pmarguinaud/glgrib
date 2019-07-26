#include "glgrib_shell.h"

#include <iostream>
#include <exception>

#include <readline/readline.h>
#include <readline/history.h>


#define C(name,code,...) \
  glgrib_command ([] (glgrib_window * gwindow, \
                      glgrib_shell * shell,    \
		      glgrib_command_arghash & args) \
		      {code}, #name, __VA_ARGS__)
#define A(...) glgrib_command_arg (__VA_ARGS__)
#define def(name) \
static void command_##name (glgrib_window * gwindow, class glgrib_shell * shell, \
                            glgrib_command_arghash & args)

#define F(x) std::stof (args.at (#x).value)
#define I(x) std::stoi (args.at (#x).value)
#define S(x)           (args.at (#x).value)

glgrib_shell Shell 
(
  C (close,     { shell->close = 1; },                            ""),
  C (fov,       { gwindow->scene.d.view.opts.fov += F (dfov);  }, "", A ("dfov", "+1.")),
  C (rotate,    { gwindow->scene.d.rotate_earth = ! gwindow->scene.d.rotate_earth; }, ""),
  C (wireframe, { gwindow->toggle_wireframe (); }, ""),
  C (flat,      { gwindow->toggle_flat (); }, ""),
  C (select,    { shell->windowid = I (id); }, "", A ("id", "1")),
  C (remove,    { gwindow->remove_field (I (rank)); }, "", A ("rank", "0")),
  C (load,      
	  { 
            glgrib_options_field opts; 
	    std::cout << S (path) << std::endl;
            opts.path.push_back (S (path)); 
            opts.scale = F (scale);
            opts.palette = S (palette);
            gwindow->load_field (opts, I (rank)); 
	  }, "", 
     A ("palette", ""), A ("scale", "1.02"), A ("path", ""), A ("rank", "0")),
  C (palette,    
	  {
            gwindow->set_field_palette (S (name));
	  }, "",
     A ("name", "")),
  C (palette_min,    
	  {
            gwindow->set_field_palette_min (F (min));
	  }, "",
     A ("min", "-1.")),
  C (palette_max,    
	  {
            gwindow->set_field_palette_max (F (max));
	  }, "",
     A ("max", "+1."))
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

void glgrib_shell::execute (const std::string & _line, glgrib_window * gwindow)
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
      Cmd.func (gwindow, this, h);
    }
  catch (std::exception & e)
    {
      std::cout << "An error occurred " << std::endl;
      std::cout << e.what() << std::endl;
      std::cout << Cmd.help () << std::endl;
    }
}

static 
void * _run (void * data)
{
  glgrib_shell * shell = (glgrib_shell *)data;
  shell->run ();
  return NULL;
}


void glgrib_shell::start (glgrib_window_set * ws)
{
  wset = ws;
  pthread_create(&thread, NULL, _run, this);
}


void glgrib_shell::run ()
{
  if (read_history (".glgrib_history") != 0)
    write_history (".glgrib_history");
  while (wset->size () > 0)
    {
      char * line = readline ("glgrib> ");
      if (line == NULL)
        break;
      if (strlen (line) > 0) 
        add_history (line);
 
      lock ();
      {
        if (wset->size ())
          {
            glgrib_window * gwindow = wset->getWindowById (windowid);
	    if (gwindow == NULL)
              gwindow = wset->getFirstWindow ();
            execute (line, gwindow);
	  }
      }
      unlock ();
      
      append_history (1, ".glgrib_history");

      free (line);
      
      if (closed ()) 
        break;

    }
}


