#include "glgrib_options.h"
#include "glgrib_resolve.h"
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <map>


std::string glgrib_option_date::asString () const 
{
  char tmp[128];
  sprintf (tmp, "%4.4ld/%2.2ld/%2.2ld %2.2ld:%2.2ld:%2.2ld", year, month, day, hour, minute, second);
  return std::string (tmp);
}

glgrib_option_date glgrib_option_date::date_from_t (time_t time)
{
  struct tm t;
  glgrib_option_date d;
  memset (&t, 0, sizeof (t));
  gmtime_r (&time, &t);
  d.second = t.tm_sec;
  d.minute = t.tm_min;
  d.hour   = t.tm_hour;
  d.day    = t.tm_mday;
  d.month  = t.tm_mon + 1;
  d.year   = t.tm_year + 1900;
  return d;
}

time_t glgrib_option_date::t_from_date (const glgrib_option_date & d)
{
  time_t time;
  struct tm t;                  
  memset (&t, 0, sizeof (t));
  t.tm_sec    = d.second;       
  t.tm_min    = d.minute;       
  t.tm_hour   = d.hour;         
  t.tm_mday   = d.day;          
  t.tm_mon    = d.month - 1;    
  t.tm_year   = d.year - 1900;  
  return timegm (&t);             
}

glgrib_option_date glgrib_option_date::interpolate 
(
  const glgrib_option_date & d1, const glgrib_option_date & d2, const float alpha
)
{
  return date_from_t (round ((double)alpha * t_from_date (d1) + (1.0 - (double)alpha) * t_from_date (d2)));
}

void glgrib_option_color::parse (int * count, glgrib_option_color * value, const char * v)
{
  if ((*count == 0) && (v[0] == '#') && (strlen (v) == 7 || strlen (v) == 9))
    {
      *value = color_by_hexa (v);
    }
  else if ((*count == 0) && (! isdigit (v[0])))
    {
      *value = color_by_name (v);
    }
  else
    {
      int c = std::stoi (v);
      switch (*count)
        {
          case 0: value->r = c; break;
          case 1: value->g = c; break;
          case 2: value->b = c; break;
        }
      (*count)++;
    }
}

glgrib_option_color glgrib_option_color::color_by_hexa (const char * name)
{
  glgrib_option_color color;
  if (strlen (name) == 7)
    {
      if (sscanf (name, "#%2x%2x%2x", &color.r, &color.g, &color.b) != 3)
        goto error;
    }
  else if (strlen (name) == 9)
    {
      if (sscanf (name, "#%2x%2x%2x%2x", &color.r, &color.g, &color.b, &color.a) != 4)
        goto error;
    }
  else
    {
      goto error;
    }
  return color;

error:
  throw std::runtime_error ("Cannot parse hexa color");
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

glgrib_option_color glgrib_option_color::color_by_name (const char * n)
{
  glgrib_option_color color;

  int len = strlen (n);
  char name[len+1];

  if ((len > 3) && (n[len-3] == '#') && isalpha (n[len-2]) && isalpha (n[len-1]))
    {
      strncpy (name, n, len-3);
      name[len-3] = '\0';
      sscanf (name+len-2, "%2x", &color.a);
    }
  else
    {
      strcpy (name, n);
    }


  name2hexa_t::const_iterator it = name2hexa.find (std::string (name));
  if (it != name2hexa.end ())
    return color_by_hexa (it->second.c_str ());

  sqlite3 * db = NULL;
  sqlite3_stmt * req = NULL;
  int rc;
  bool found = false;

#define TRY(expr) do { if ((rc = expr) != SQLITE_OK) goto end; } while (0)
  TRY (sqlite3_open (glgrib_resolve ("glgrib.db").c_str (), &db));
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

          if (arg == std::string ("}-")) // Close option group
            {
              ctx.pop_back ();
            }
          else if (arg.substr (0, 2) == std::string ("--"))
            {
              int len = arg.length ();
              if (arg.substr (len-2, 2) == std::string ("-{")) // Start option group
                {
                  ctx.push_back (arg.substr (2, len-4));
                }
              else
                {

                  if (ctx.size () > 0) // We have an active option group; prepend its name to the option being processed
                    {
                      std::string a = arg.substr (2);
                      arg = "";
                      for (int i = 0; i < ctx.size (); i++)
                        if (i == 0)
                          arg = ctx[i];
                        else
                          arg = arg + "." + ctx[i];
                      arg = "--" + arg + "." + a;
                    }
                  if (arg == "--help")
                    {
                      show_help ();
                      return false;
                    }

                  bool found = name2option.find (arg) != name2option.end ();

                  if (found) // Set option
                    {
                      seen.insert (arg);
                      opt = name2option[arg];
                      opt->clear (); // Clear option; this means that if options appears several times, then the last setting is taken into account
                      if (! opt->has_arg ())
                        opt->set (NULL);
                    }
                  else if (arg.substr (arg.length () - 4, 4) == ".off")
                    {
                      std::string a = arg;
                      a.replace (a.length () - 4, 4, ".on"); 
                      found = name2option.find (a) != name2option.end ();
                      if (found) // Unset option
                        {
                          seen.insert (a);
                          opt = name2option[a];
                          opt->clear (); // Set boolean option to false
                        }
                    }

                  if (! found)
                    {
                      throw std::runtime_error (std::string ("Unknown option ") + arg);
                    }
                }
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

bool glgrib_options_parser::seenOption (const std::string & name) const
{
  for (std::set<std::string>::const_iterator it = seen.begin (); it != seen.end (); it++)
    {
      const std::string & n = *it;
      if (n.substr (0, name.length ()) == name)
        return true;
    }
  return false;
}

void glgrib_options_parser::show_help ()
{
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
  return v;
}

