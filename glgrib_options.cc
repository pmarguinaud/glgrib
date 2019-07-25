#include "glgrib_options.h"
#include <sqlite3.h>
#include <string.h>
#include <map>


glgrib_option_color glgrib_option_color::color_by_hexa (const char * name)
{
  glgrib_option_color color;
  if (sscanf (name, "#%2x%2x%2x", &color.r, &color.g, &color.b) != 3)
    throw std::runtime_error ("Cannot parse hexa color");
  return color;
}

typedef std::map<std::string,std::string> name2hexa_t;
static name2hexa_t name2hexa = 
{
   {"red",   "#ff0000"},
   {"green", "#00ff00"},
   {"blue",  "#0000ff"},
   {"white", "#ffffff"},
   {"black", "#000000"}
};

glgrib_option_color glgrib_option_color::color_by_name (const char * name)
{
  glgrib_option_color color;
  
  name2hexa_t::const_iterator it = name2hexa.find (std::string (name));
  if (it != name2hexa.end ())
    return color_by_hexa (it->second.c_str ());

  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;
  bool found = false;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
  TRY (sqlite3_open (".glgrib.db", &db));
  TRY (sqlite3_prepare_v2 (db, "SELECT hexa FROM COLORS WHERE name = ?;", -1, &req, 0));
  TRY (sqlite3_bind_text (req, 1, name, strlen (name), NULL));

  if ((rc = sqlite3_step (req)) == SQLITE_ROW)
    {
      found = true;
      char hexa[8];
      strcpy (hexa, (const char *)sqlite3_column_text (req, 0));
      if (sscanf (hexa, "#%2x%2x%2x", &color.r, &color.g, &color.b) != 3)
        throw std::runtime_error ("Cannot parse hexa color");
      rc = SQLITE_OK;
      name2hexa.insert (std::pair<std::string,std::string>(std::string (name), std::string (hexa)));
    }
  else
    {
      throw std::runtime_error (std::string ("Cannot find color `") + std::string (name) + std::string ("'"));
    }

#undef TRY

end:

  if (rc != SQLITE_OK)
    printf ("%s\n", sqlite3_errmsg (db));

  if (req != NULL)
    sqlite3_finalize (req);
  if (db != NULL)
    sqlite3_close (db);

  return color;
}

bool glgrib_options_parser::parse (int argc, char * argv[])
{
  try
    {
      option_base * opt = NULL;
      for (int iarg = 1; iarg < argc; iarg++)
        {
          std::string arg (argv[iarg]);
          if (name2option.find (arg) != name2option.end ())
            {
              opt = name2option[arg];
              if (! opt->has_arg ())
                opt->set (NULL);
            }
          else if (arg == "--help")
            {
              show_help ();
              return false;
            }
          else if (arg.substr (0, 2) == std::string ("--"))
            {
              throw std::runtime_error (std::string ("Unknown option ") + arg);
            }
          else
            {
              opt->set (argv[iarg]);
            }
        }
    }
  catch (const std::exception & e)
    {
      std::cout << "Failed to parse options : " << e.what () << std::endl;
      show_help ();
      return false;
    }
  return true;
}

void glgrib_options_parser::show_help ()
{
  return;
  printf ("Usage:\n");
  size_t name_size = 0, type_size = 0;
  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    {   
      name_size = std::max (it->first.length (), name_size);
      type_size = std::max (it->second->type ().length (), type_size);
    }   
  char format[64];
  sprintf (format, " %%-%lds : %%-%lds :", name_size, type_size);
  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    {   
      option_base * opt = it->second;
      printf (format, it->first.c_str (), opt->type ().c_str ());
      printf ("      %s\n", opt->asString ().c_str ());
      printf ("      %s\n", opt->desc.c_str ());
      printf ("\n");
    }   
}

bool glgrib_options::parse (int argc, char * argv[])
{
  bool v = glgrib_options_base::parse (argc, argv);
  if (v)
    for (int j = 0; j < field.size (); j++)
      {
        if (field[j].scale.size () == 0)
          field[j].scale.push_back (1.00);
        for (int i = field[j].scale.size (); i < field[j].path.size (); i++)
          field[j].scale.push_back (field[j].scale[i-1] - 0.05);
        for (int i = field[j].palette.size (); i < field[j].path.size (); i++)
          field[j].palette.push_back ("default");
      }
  return v;
}

