#include "glGribOptions.h"
#include "glGribResolve.h"
#include "glGribPalette.h"
#include "glGribSqlite.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <map>
#include <list>

float glGribOptionsPalette::defaultMin = glGribPalette::defaultMin;
float glGribOptionsPalette::defaultMax = glGribPalette::defaultMax;
float glGribOptionsContour::defaultMin = glGribPalette::defaultMin;
float glGribOptionsContour::defaultMax = glGribPalette::defaultMax;
float glGribOptionsIsofill::defaultMin = glGribPalette::defaultMin;
float glGribOptionsIsofill::defaultMax = glGribPalette::defaultMax;

namespace glgrib_options_parser_detail
{

template <> std::string optionTmpl     <int>                ::type () { return std::string ("INTEGER"); }
template <> std::string optionTmpl     <float>              ::type () { return std::string ("FLOAT"); }
template <> std::string optionTmplList<int>                ::type () { return std::string ("LIST OF INTEGERS"); }
template <> std::string optionTmplList<float>              ::type () { return std::string ("LIST OF FLOATS"); }
template <> std::string optionTmpl     <glGribOptionDate> ::type () { return std::string ("YYYY/MM/DD_hh:mm:ss"); }
template <> std::string optionTmpl     <glGribOptionColor>::type () { return std::string ("COLOR #rrggbb(aa)"); }
template <> std::string optionTmpl     <std::string>        ::type () { return std::string ("STRING"); }
template <> std::string optionTmpl     <std::string>        ::asString () const { return '"' + *value + '"'; }
template <> std::string optionTmpl     <std::string>        ::asOption () const { return name + " " + glgrib_options_util::escape (*value); }
template <> std::string optionTmplList<glGribOptionColor>::type () { return std::string ("LIST OF COLORS #rrggbb(aa)"); }
template <> std::string optionTmplList<std::string>        ::type () { return std::string ("LIST OF STRINGS"); }
template <> std::string optionTmplList<std::string>        ::asString () const 
{ 
  std::string str; 
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); it++)
    str = str + " " + *it;
  return str;
}
template <> std::string optionTmplList<std::string>        ::asOption () const 
{ 
  std::string str = name + " ";
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); it++)
    str = str + " " + glgrib_options_util::escape (*it);
  return str;
}
template <> std::string optionTmpl     <bool>               ::type () { return std::string ("BOOLEAN"); }

template <> void optionTmpl<bool>::set ()
{
  *value = true;
}

template <> void optionTmpl<std::string>::set (const std::string & v)
{
  *value = std::string (v);
}

template <> void optionTmplList<std::string>::set (const std::string & v)
{
  value->push_back (std::string (v));
}

template <> void optionTmpl<bool>::clear ()
{
  if (value != nullptr) 
    *value = false; 
}

template <> std::string optionTmpl<bool>::asString () const
{
  return *value ? std::string ("TRUE") : std::string ("FALSE");
}

template <> std::string optionTmpl<bool>::asOption () const
{
  std::string val = name;
  val.replace (val.length () - 3, 3, (*value ? ".on" : ".off"));
  return val;
}

template <> int optionTmpl<bool>::hasArg () const
{
  return 0;
}

};

std::ostream & operator << (std::ostream & out, const glGribOptionDate & date)
{
  return out << date.asString ();
}

std::istream & operator >> (std::istream & in, glGribOptionDate & date)
{
  std::string str;
  in >> str;
  glGribOptionDate::parse (&date, str.c_str ());
  return in;
}

void glGribOptionDate::parse (glGribOptionDate * date, const std::string & v)
{
  sscanf (v.c_str (), "%4ld/%2ld/%2ld_%2ld:%2ld:%2ld", &date->year, 
          &date->month, &date->day, &date->hour, &date->minute, &date->second);
}

std::string glGribOptionDate::asString () const 
{
  char tmp[128];
  sprintf (tmp, "%4.4ld/%2.2ld/%2.2ld_%2.2ld:%2.2ld:%2.2ld", year, month, day, hour, minute, second);
  return std::string (tmp);
}

glGribOptionDate glGribOptionDate::date_from_t (time_t time)
{
  struct tm t;
  glGribOptionDate d;
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

time_t glGribOptionDate::t_from_date (const glGribOptionDate & d)
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

glGribOptionDate glGribOptionDate::interpolate 
(
  const glGribOptionDate & d1, const glGribOptionDate & d2, const float alpha
)
{
  return date_from_t (round ((double)alpha * t_from_date (d1) + (1.0 - (double)alpha) * t_from_date (d2)));
}

std::ostream & operator << (std::ostream & out, const glGribOptionColor & color)
{
  return out << color.asString ();
}

std::istream & operator >> (std::istream & in, glGribOptionColor & color)
{
  std::string str;
  in >> str;
  glGribOptionColor::parse (&color, str.c_str ());
  return in;
}

void glGribOptionColor::parse (glGribOptionColor * value, const std::string & v)
{
  if ((v[0] == '#') && (v.length () == 7 || v.length () == 9))
    {
      *value = colorByHexa (v);
    }
  else 
    {
      *value = colorByName (v);
    }
}

glGribOptionColor::glGribOptionColor (const std::string & str)
{
  parse (this, str);
}

glGribOptionColor glGribOptionColor::colorByHexa (const std::string & name)
{
  glGribOptionColor color;
  if (name.length () == 7)
    {
      if (sscanf (name.c_str (), "#%2x%2x%2x", &color.r, &color.g, &color.b) != 3)
        goto error;
    }
  else if (name.length () == 9)
    {
      if (sscanf (name.c_str (), "#%2x%2x%2x%2x", &color.r, &color.g, &color.b, &color.a) != 4)
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

glGribOptionColor glGribOptionColor::colorByName (const std::string & n)
{
  glGribOptionColor color;

  int len = n.length ();
  std::string name;

  if ((len > 3) && (n[len-3] == '#') && isalpha (n[len-2]) && isalpha (n[len-1]))
    {
      name = n.substr (0, len-3);
      sscanf (name.substr (len-2).c_str (), "%2x", &color.a);
    }
  else
    {
      name = n;
    }

  name2hexa_t::const_iterator it = name2hexa.find (std::string (name));
  if (it != name2hexa.end ())
    return colorByHexa (it->second.c_str ());

  glGribSqlite db (glGribResolve ("glGrib.db"));

  glGribSqlite::stmt st = db.prepare ("SELECT hexa FROM COLORS WHERE name = ?;");
  st.bindall (&name);

  std::string hexa;
  if (st.fetchRow (&hexa))
    {
      if (sscanf (&hexa[0], "#%2x%2x%2x", &color.r, &color.g, &color.b) != 3)
        throw std::runtime_error ("Cannot parse hexa color");
      name2hexa.insert (std::pair<std::string,std::string>(std::string (name), hexa));
    }
  else
    {
      throw std::runtime_error (std::string ("Cannot find color `") + std::string (name) + std::string ("'"));
    }

  return color;
}

std::string glgrib_options_util::escape (const std::string & token)
{ 
  std::string tok;

  for (int i = 0; i < token.length (); i++)
    {
      if (token[i] == '"') 
        tok.push_back ('\\');
      if (token[i] == '\\') 
        tok.push_back ('\\');
      tok.push_back (token[i]);
    }

  return '"' + tok + '"';
}


std::string glgrib_options_util::nextToken (std::string * line)
{
  while (line->length () && (*line)[0] == ' ')
    *line = line->substr (1);

  std::string token = std::string ("");

  int q = 0, qq = 0;

  while (line->length ())
    {
      char c = (*line)[0];
      *line = line->substr (1);


      if ((qq == 0) && (q == 0))
        {
          if (c == ' ')
            break;
          if (c == '"')
            {
              qq = 1;
              goto cont;
            }
          if (c == '\'')
            {
              q = 1;
              goto cont;
            }
        }
      else
        {
          if ((c == '"') && (qq == 1))
            {
              qq = 0;
              goto cont;
            }
          if ((c == '\'') && (q == 1))
            {
              q = 0;
              goto cont;
            }
        }

      if (c == '\\')
        {
          if (line->length ())
            {
              c = (*line)[0];
              *line = line->substr (1);
            }
          else
            throw std::runtime_error (std::string ("Stray '\\'"));
        }
      token.push_back (c);

cont:
      continue;
    }

  if (qq || q)
    throw std::runtime_error (std::string ("Unterminated character string"));

  return token;
}

bool glGribOptionsParser::parse (int _argc, const char * _argv[], 
		                   const std::set<std::string> * skip)
{
  int argc = _argc;
  bool do_skip = false;

  std::list<std::string> argv;
  for (int i = 1; i < argc; i++)
    argv.push_back (_argv[i]);
  try
    {
      glgrib_options_parser_detail::optionBase * opt = nullptr;

      while (argv.size () > 0)
        {
          std::list<std::string>::iterator it = argv.begin ();
          std::string arg = *it;
          argv.pop_front ();
          int len = arg.length ();

          // Options read from file
          if ((len > 4) && (arg.substr (0, 3) == "--{") && (arg[len-1] == '}'))
            {
              const std::string include = arg.substr (3, len-4);
              std::list<std::string> ll;
              std::ifstream fp (include);
              if (! fp)
                throw std::runtime_error (std::string ("Cannot open file `") 
                      + include + std::string ("' for reading"));
              std::string line;
              while (std::getline (fp, line))
                {
                  while (1)
                    {
                      std::string token = glgrib_options_util::nextToken (&line);
                      if (token == "")
                        break;
                      ll.push_back (token);
                    }
                }
              while (ll.size ())
                {
                  argv.push_front (ll.back ());
                  ll.pop_back ();
                }

              continue;
            }
          else if (arg == std::string ("}-")) // Close option group
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
                      do_skip = skip && (skip->find (arg) != skip->end ());

                      seen.insert (arg);
                      opt = name2option[arg];
		      if (! do_skip)
		        {
                          opt->clear (); // Clear option; this means that if options appears several times, then the last setting is taken into account
                          if (! opt->hasArg ())
                            opt->set ();
			}
                    }
                  else if (arg.substr (arg.length () - 4, 4) == ".off")
                    {
                      std::string a = arg;
                      a.replace (a.length () - 4, 4, ".on"); 
                      found = name2option.find (a) != name2option.end ();
                      if (found) // Unset option
                        {
                          do_skip = skip && (skip->find (a) != skip->end ());
                          seen.insert (a);
                          opt = name2option[a];
			  if (! do_skip)
                            opt->clear (); // Set boolean option to false
                        }
                    }

                  if (! found)
                    throw std::runtime_error (std::string ("Unknown option ") + arg);
                }
            }
          else if (opt == nullptr)
            {
              throw std::runtime_error (std::string ("Error parsing options: no valid option for `") + arg + std::string ("'"));
            }
          else if (! do_skip)
            {
              opt->set (arg);
            }

        }
    }
  catch (const std::exception & e)
    {
      std::cout << "Failed to parse options : " << e.what () << std::endl;
      return false;
    }



  return true;
}

bool glGribOptionsBase::parse (int argc, const char * argv[], 
		                 const std::set<std::string> * skip)
{
  glGribOptionsParser p;
  traverse ("", &p);
  return p.parse (argc, argv, skip);
}

bool glGribOptionsBase::parse (const char * args,
		                 const std::set<std::string> * skip)
{
  std::string line (args);
  std::vector<std::string> list;
 
  while (1)
    {
      std::string token = glgrib_options_util::nextToken (&line);
      if (token == "")
        break;
      list.push_back (token);
    }

  int argc = 1 + list.size ();
  const char * argv [argc];

  for (int i = 0; i < list.size (); i++)
    argv[1+i] = list[i].c_str ();

  glGribOptionsParser p;
  traverse ("", &p);
  return p.parse (argc, argv, skip);
}

bool glGribOptions::parse (int argc, const char * argv[], 
		            const std::set<std::string> * skip)
{
  glGribOptionsParser p;
  traverse ("", &p);
  if (! p.parse (argc, argv, skip))
    return false;

  std::set<std::string> seen = p.getSeenOptions ();

  // Should go in method like postProcessOption
  for (int i = 0; i < field.size (); i++)
    {
      std::string prefix = "--field[" +  std::to_string (i) + "]";
      for (std::set<std::string>::iterator it = seen.begin (); it != seen.end (); it++)
        if (it->substr (0, prefix.length ()) == prefix)
          {
            std::string str = *it;
	    str.replace (0, 1 + prefix.length (), "");
	    str = "--" + str;
            field[i].seen.insert (str);
	  }
    }

  return true;
}

bool glGribOptionsParser::seenOption (const std::string & name) const
{
  for (std::set<std::string>::const_iterator it = seen.begin (); it != seen.end (); it++)
    {
      const std::string & n = *it;
      if (n.substr (0, name.length ()) == name)
        return true;
    }
  return false;
}

void glGribOptionsParser::show_help ()
{
  printf ("Usage:\n");
  display (std::string ("--"));
}

std::string glGribOptionsParser::asOption (glGribOptionsParser & p2) 
{
  std::string str;

  for (name2option_t::const_iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    {
	glgrib_options_parser_detail::optionBase * opt1 = it->second;
	glgrib_options_parser_detail::optionBase * opt2 = p2.name2option[it->first];

	std::string str1 = opt1->asOption ();
	std::string str2 = opt2->asOption ();

	if (str1 != str2)
	  str = str + str1 + " ";
    }

  return str;
}

void glGribOptionsParser::display (const std::string & prefix, bool show_hidden)
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
	glgrib_options_parser_detail::optionBase * opt = it->second;
        if ((! show_hidden) && (opt->hidden))
          continue;
        printf (format, it->first.c_str (), opt->type ().c_str ());
        printf ("      %s\n", opt->asString ().c_str ());
        printf ("      %s\n", opt->desc.c_str ());
        printf ("\n");
      }   
}

void glGribOptionsParser::print (glGribOptions & opts1)
{
  glGribOptionsParser p1, p2;
  glGribOptions opts2;

  opts1.traverse ("", &p1);
  opts2.traverse ("", &p2);

  std::vector<std::string> options_list;
  p1.getOptions (&options_list);


  for (int i = 0; i < options_list.size (); i++)
    {
      const std::string & name = options_list[i];
      const glgrib_options_parser_detail::optionBase * o1 = p1.getOption (name);
      const glgrib_options_parser_detail::optionBase * o2 = p2.getOption (name);
      if (o1->isEqual (o2))
        continue;
      std::cout << "  " << name;
      if (o1->hasArg ())
        std::cout << " " << o1->asString ();
      std::cout << std::endl;
    }


}

std::string glGribOptionsBase::asOption (glGribOptionsBase & ref) 
{
  glGribOptionsParser p1, p2;
  traverse ("", &p1);
  ref.traverse ("", &p2);
  return p1.asOption (p2);
}


bool glGribOptionsField::parse_unseen (const char * args)
{
  parse (args, &seen);
  return true;
}


