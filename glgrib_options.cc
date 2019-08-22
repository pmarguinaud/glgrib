#include "glgrib_options.h"
#include "glgrib_resolve.h"
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <map>


namespace glgrib_parser_ns
{

template <> std::string option_tmpl     <int>                ::type () { return std::string ("INTEGER"); }
template <> std::string option_tmpl     <float>              ::type () { return std::string ("FLOAT"); }
template <> std::string option_tmpl_list<int>                ::type () { return std::string ("LIST OF INTEGERS"); }
template <> std::string option_tmpl_list<float>              ::type () { return std::string ("LIST OF FLOATS"); }
template <> std::string option_tmpl     <glgrib_option_date> ::type () { return std::string ("YYYY/MM/DD_hh:mm:ss"); }
template <> std::string option_tmpl     <glgrib_option_color>::type () { return std::string ("COLOR #rrggbb(aa)"); }
template <> std::string option_tmpl     <std::string>        ::type () { return std::string ("STRING"); }
template <> std::string option_tmpl     <std::string>        ::asString () const { return '"' + *value + '"'; }
template <> std::string option_tmpl_list<glgrib_option_color>::type () { return std::string ("LIST OF COLORS #rrggbb(aa)"); }
template <> std::string option_tmpl_list<std::string>        ::type () { return std::string ("LIST OF STRINGS"); }
template <> std::string option_tmpl_list<std::string>        ::asString () const 
{ 
  std::string str; 
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); it++)
    str = str + " " + '"' + *it + '"';
  return str;
}
template <> std::string option_tmpl     <bool>               ::type () { return std::string ("BOOLEAN"); }

template <> void option_tmpl<bool>::set (const char * v)
{
  if (v != NULL)
    throw std::runtime_error (std::string ("Option ") + name + std::string (" does not take any value"));
  *value = true;
}

template <> void option_tmpl<bool>::clear ()
{
  if (value != NULL) 
    *value = false; 
}

template <> std::string option_tmpl<bool>::asString () const
{
  return *value ? std::string ("TRUE") : std::string ("FALSE");
}

template <> int option_tmpl<bool>::has_arg () const
{
  return 0;
}

};

std::ostream & operator << (std::ostream & out, const glgrib_option_date & date)
{
  return out << date.asString ();
}

std::istream & operator >> (std::istream & in, glgrib_option_date & date)
{
  std::string str;
  in >> str;
  glgrib_option_date::parse (&date, str.c_str ());
  return in;
}

void glgrib_option_date::parse (glgrib_option_date * date, const char * v)
{
  sscanf (v, "%4ld/%2ld/%2ld_%2ld:%2ld:%2ld", &date->year, 
          &date->month, &date->day, &date->hour, &date->minute, &date->second);
}

std::string glgrib_option_date::asString () const 
{
  char tmp[128];
  sprintf (tmp, "%4.4ld/%2.2ld/%2.2ld_%2.2ld:%2.2ld:%2.2ld", year, month, day, hour, minute, second);
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

std::ostream & operator << (std::ostream & out, const glgrib_option_color & color)
{
  return out << color.asString ();
}

std::istream & operator >> (std::istream & in, glgrib_option_color & color)
{
  std::string str;
  in >> str;
  glgrib_option_color::parse (&color, str.c_str ());
  return in;
}

void glgrib_option_color::parse (glgrib_option_color * value, const char * v)
{
  if ((v[0] == '#') && (strlen (v) == 7 || strlen (v) == 9))
    {
      *value = color_by_hexa (v);
    }
  else 
    {
      *value = color_by_name (v);
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

bool glgrib_options_parser::parse (int argc, const char * argv[])
{
  try
    {
      glgrib_parser_ns::option_base * opt = NULL;

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
                    throw std::runtime_error (std::string ("Unknown option ") + arg);
                }
            }
          else if (opt == NULL)
            {
              throw std::runtime_error (std::string ("Error parsing options: no valid option for `") + arg + std::string ("'"));
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
  display (std::string ("--"));
}

void glgrib_options_parser::display (const std::string & prefix)
{
  size_t name_size = 0, type_size = 0;
  int len = prefix.size ();

  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    if (it->first.substr (0, len) == prefix)
      {   
        name_size = std::max (it->first.length (), name_size);
        type_size = std::max (it->second->type ().length (), type_size);
      }   
  char format[64];
  sprintf (format, " %%-%lds : %%-%lds :", name_size, type_size);
  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    if (it->first.substr (0, len) == prefix)
      {   
	glgrib_parser_ns::option_base * opt = it->second;
        printf (format, it->first.c_str (), opt->type ().c_str ());
        printf ("      %s\n", opt->asString ().c_str ());
        printf ("      %s\n", opt->desc.c_str ());
        printf ("\n");
      }   
}

bool glgrib_options::parse (int argc, const char * argv[])
{
  bool v = glgrib_options_base::parse (argc, argv);
  return v;
}

void glgrib_parser_ns::set_print (glgrib_options & opts1)
{
  glgrib_options_parser p1, p2;
  glgrib_options opts2;

  opts1.traverse ("", &p1);
  opts2.traverse ("", &p2);

  std::vector<std::string> options_list;
  p1.getOptions (&options_list);


  for (int i = 0; i < options_list.size (); i++)
    {
      const std::string & name = options_list[i];
      const glgrib_parser_ns::option_base * o1 = p1.getOption (name);
      const glgrib_parser_ns::option_base * o2 = p2.getOption (name);
      if (o1->isEqual (o2))
        continue;
      std::cout << "  " << name;
      if (o1->has_arg ())
        std::cout << " " << o1->asString ();
      std::cout << std::endl;
    }


}


