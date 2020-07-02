#include "glGribOptions.h"
#include "glGribResolve.h"
#include "glGribPalette.h"
#include "glGribSQLite.h"
#include "glGribGrok.h"

#include <cstring>
#include <time.h>
#include <cmath>
#include <map>
#include <list>

namespace glGrib::OptionsParserDetail
{

template <> const std::string optionTmpl    <int>                 ::type () { return std::string ("INTEGER"); }
template <> const std::string optionTmpl    <float>               ::type () { return std::string ("FLOAT"); }
template <> const std::string optionTmplList<int>                 ::type () { return std::string ("INTEGER-LIST"); }
template <> const std::string optionTmplList<float>               ::type () { return std::string ("FLOAT-LIST"); }
template <> const std::string optionTmpl     <glGrib::OptionDate> ::type () { return std::string ("DATE"); }
template <> const std::string optionTmpl     <glGrib::OptionColor>::type () { return std::string ("COLOR"); }
template <> const std::string optionTmpl     <std::string>        ::type () { return std::string ("STRING"); }
template <> const std::string optionTmpl     <std::string>        ::asString () const { return *value; }
template <> const std::string optionTmpl     <std::string>        ::asJSON   () const { return glGrib::OptionsUtil::escape (*value); }
template <> const std::string optionTmpl     <std::string>        ::asOption () const { return name + " " + glGrib::OptionsUtil::escape (*value); }
template <> const std::string optionTmplList<glGrib::OptionColor> ::type () { return std::string ("COLOR-LIST"); }
template <> const std::string optionTmplList<std::string>         ::type () { return std::string ("STRING-LIST"); }
template <> const std::string optionTmplList<std::string>         ::asString () const 
{ 
  std::string str; 
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); ++it)
    str = str + " " + *it;
  return str;
}
template <> const std::string optionTmplList<std::string>         ::asJSON () const 
{ 
  std::string json; 
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); ++it)
    {
      if (it != value->begin ())
        json += ",";
      json += glGrib::OptionsUtil::escape (*it);
    }
  return std::string ("[") + json + std::string ("]");
}
template <> const std::string optionTmplList<std::string>        ::asOption () const 
{ 
  std::string str = name + " ";
  for (std::vector<std::string>::const_iterator it = value->begin (); it != value->end (); ++it)
    str = str + " " + glGrib::OptionsUtil::escape (*it);
  return str;
}
template <> const std::string optionTmpl     <bool>               ::type () { return std::string ("BOOLEAN"); }

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

template <> const std::string optionTmpl<bool>::asString () const
{
  return *value ? std::string ("TRUE") : std::string ("FALSE");
}

template <> const std::string optionTmpl<bool>::asJSON () const
{
  return *value ? std::string ("true") : std::string ("false");
}

template <> const std::string optionTmpl<bool>::asOption () const
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

namespace glGrib
{

std::ostream & operator << (std::ostream & out, const glGrib::OptionDate & date)
{
  return out << date.asString ();
}

std::istream & operator >> (std::istream & in, glGrib::OptionDate & date)
{
  std::string str;
  in >> str;
  glGrib::OptionDate::parse (&date, str.c_str ());
  return in;
}

}

void glGrib::OptionDate::parse (glGrib::OptionDate * date, const std::string & v)
{
  sscanf (v.c_str (), "%4ld/%2ld/%2ld_%2ld:%2ld:%2ld", &date->year, 
          &date->month, &date->day, &date->hour, &date->minute, &date->second);
}

const std::string glGrib::OptionDate::asString () const 
{
  char tmp[128];
  sprintf (tmp, "%4.4ld/%2.2ld/%2.2ld_%2.2ld:%2.2ld:%2.2ld", year, month, day, hour, minute, second);
  return std::string (tmp);
}

glGrib::OptionDate glGrib::OptionDate::date_from_t (time_t time)
{
  struct tm t;
  glGrib::OptionDate d;
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

time_t glGrib::OptionDate::tFromDate (const glGrib::OptionDate & d)
{
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

glGrib::OptionDate glGrib::OptionDate::interpolate 
(
  const glGrib::OptionDate & d1, const glGrib::OptionDate & d2, const float alpha
)
{
  return date_from_t (round (static_cast<double> (alpha) * tFromDate (d1) + (1.0 - static_cast<double> (alpha)) * tFromDate (d2)));
}

namespace glGrib
{

std::ostream & operator << (std::ostream & out, const glGrib::OptionColor & color)
{
  return out << color.asString ();
}

std::istream & operator >> (std::istream & in, glGrib::OptionColor & color)
{
  std::string str;
  in >> str;
  glGrib::OptionColor::parse (&color, str.c_str ());
  return in;
}

}

void glGrib::OptionColor::parse (glGrib::OptionColor * value, const std::string & v)
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

glGrib::OptionColor::OptionColor (const std::string & str)
{
  parse (this, str);
}

glGrib::OptionColor glGrib::OptionColor::colorByHexa (const std::string & name)
{
  glGrib::OptionColor color;
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

namespace
{
typedef std::map<std::string,std::string> name2hexa_t;
name2hexa_t name2hexa = 
{
   {"red",   "#ff0000"},
   {"green", "#00ff00"},
   {"blue",  "#0000ff"},
   {"white", "#ffffff"},
   {"black", "#000000"}
};
}

glGrib::OptionColor glGrib::OptionColor::colorByName (const std::string & n)
{
  glGrib::OptionColor color;

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

  glGrib::SQLite db (glGrib::Resolve ("glGrib.db"));

  glGrib::SQLite::stmt st = db.prepare ("SELECT hexa FROM COLORS WHERE name = ?;");
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

const std::string glGrib::OptionsUtil::escape (const std::string & token)
{ 
  std::string tok;

  for (size_t i = 0; i < token.length (); i++)
    {
      if (token[i] == '"') 
        tok.push_back ('\\');
      if (token[i] == '\\') 
        tok.push_back ('\\');
      tok.push_back (token[i]);
    }

  return '"' + tok + '"';
}


const std::string glGrib::OptionsUtil::nextToken (std::string * line)
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

bool glGrib::OptionsParser::parse (int _argc, const char * _argv[], 
		                   const std::set<std::string> * skip)
{
  int argc = _argc;
  bool do_skip = false;

  std::list<std::string> argv;
  for (int i = 1; i < argc; i++)
    argv.push_back (_argv[i]);
  try
    {
      glGrib::OptionsParserDetail::optionBase * opt = nullptr;

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
                      std::string token = glGrib::OptionsUtil::nextToken (&line);
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
                      for (size_t i = 0; i < ctx.size (); i++)
                        if (i == 0)
                          arg = ctx[i];
                        else
                          arg = arg + "." + ctx[i];
                      arg = "--" + arg + "." + a;
                    }

                  if (arg == "--json")
                    {
                      showJSON ();
                      return false;
                    }

                  if (arg == "--help")
                    {
                      showHelpShort ();
                      return false;
                    }

                  if (arg == "--help-long")
                    {
                      showHelpLong ();
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

bool glGrib::OptionsBase::parse (int argc, const char * argv[], 
		                 const std::set<std::string> * skip)
{
  glGrib::OptionsParser p;
  traverse ("", &p);
  return p.parse (argc, argv, skip);
}

bool glGrib::OptionsBase::parse (const char * args,
		                 const std::set<std::string> * skip)
{
  std::string line (args);
  std::vector<std::string> list;
 
  while (1)
    {
      std::string token = glGrib::OptionsUtil::nextToken (&line);
      if (token == "")
        break;
      list.push_back (token);
    }

  int argc = 1 + list.size ();
  const char * argv [argc];

  for (size_t i = 0; i < list.size (); i++)
    argv[1+i] = list[i].c_str ();

  glGrib::OptionsParser p;
  traverse ("", &p);
  return p.parse (argc, argv, skip);
}

bool glGrib::Options::parse (int argc, const char * argv[], 
		             const std::set<std::string> * skip)
{
  auto & opts = *this;

  if ((argc == 2) && strncmp (argv[1], "--", 2))
    {
      bool simple = false;
      switch (glGrib::Grok (argv[1]))
        {
          case glGrib::grok_t::UNKNOWN:
            opts.shell.on = true;
            opts.shell.script = std::string (argv[1]);
            break;
          case glGrib::grok_t::LFI:
            opts.review.on = true;
            opts.review.path = std::string (argv[1]);
            opts.scene.title.on = true;
            simple = true;
            break;
          case glGrib::grok_t::LFI_EXT:
          case glGrib::grok_t::GRIB:
          case glGrib::grok_t::GRIB_EXT:
            opts.field[0].path.push_back (std::string (argv[1]));
            simple = true;
            break;
        }
      if (simple)
        {
          opts.colorbar.on = true;
          opts.grid.on = true;
          opts.coast.on = true;
          opts.render.width = opts.render.height * 1.4;
          opts.scene.center.on = true;
        }
      return true;
    }
  else if ((argc == 4) && (strcmp (argv[1], "--diff") == 0))
    {
      opts.diff.on = true;
      opts.colorbar.on = true;
      opts.grid.on = true;
      opts.coast.on = true;
      opts.scene.title.on = true;
      opts.diff.path.push_back (std::string (argv[2]));
      opts.diff.path.push_back (std::string (argv[3]));
      return true;
    }


  glGrib::OptionsParser p;
  traverse ("", &p);
  if (! p.parse (argc, argv, skip))
    return false;

  std::set<std::string> seen = p.getSeenOptions ();

  // Should go in method like postProcessOption
  for (size_t i = 0; i < field.size (); i++)
    {
      std::string prefix = "--field[" +  std::to_string (i) + "]";
      for (std::set<std::string>::iterator it = seen.begin (); it != seen.end (); ++it)
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

bool glGrib::OptionsParser::seenOption (const std::string & name) const
{
  for (std::set<std::string>::const_iterator it = seen.begin (); 
       it != seen.end (); ++it)
    {
      const std::string & n = *it;
      if (n.substr (0, name.length ()) == name)
        return true;
    }
  return false;
}

void glGrib::OptionsParser::showJSON ()
{
  std::cout << getJSON ("--", true);
}

void glGrib::OptionsParser::showHelpShort ()
{
  std::cout << R"HELP(
Simple interface:
  
  Review (use PageUp/PageDown) : 

  $ glgrib share/data/diff/ICMSHFCST+0001

  Single field :

  $ glgrib share/data/diff/ICMSHFCST+0001%PROFTEMPERATURE
  $ glgrib share/data/ecmwf/ecmf_0_1_0_ml_137_q.grib2

  Diff :

  $ glgrib --diff  share/data/diff/ICMSHFCST+0001 share/data/diff/ICMSHFCST+0002

  
Use option --help-long to see the full interface with all options.

)HELP";
}
  
void glGrib::OptionsParser::showHelpLong ()
{
  showHelpShort ();
  std::cout << "All options:" << std::endl << std::endl;
  std::cout << getHelp ("--");
}

const std::string glGrib::OptionsParser::asOption (glGrib::OptionsParser & p2) 
{
  std::string str;

  for (name2option_t::const_iterator it = name2option.begin (); 
       it != name2option.end (); ++it)
    {
	glGrib::OptionsParserDetail::optionBase * opt1 = it->second;
	glGrib::OptionsParserDetail::optionBase * opt2 = p2.name2option[it->first];

	std::string str1 = opt1->asOption ();
	std::string str2 = opt2->asOption ();

	if (str1 != str2)
	  str = str + str1 + " ";
    }

  return str;
}

void glGrib::OptionsParser::getValue 
   (std::vector<std::string> * list, const std::string & prefix, 
    bool show_hidden, bool sort, glGrib::OptionsParser * p1)
{
  for (auto & it : filterOptions (prefix, show_hidden, sort, p1))
    {
      glGrib::OptionsParserDetail::optionBase * opt = it->second;
      if (opt->hasArg ())
        {
          list->push_back (it->first);
          list->push_back (opt->asString ());
        }
      else
        {
          list->push_back (opt->asOption ());
        }
    }

}

const std::string glGrib::OptionsParser::getHelp 
  (const std::string & prefix, bool show_hidden, bool sort)
{
  std::string help;

  size_t name_size = 0, type_size = 0;
  int len = prefix.size ();

  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); ++it)
    if (it->first.substr (0, len) == prefix)
      {   
        name_size = std::max (it->first.length (), name_size);
        type_size = std::max (it->second->type ().length (), type_size);
      }   

  char format[64];
  sprintf (format, 
           " %%-%lds : %%-%lds :"
	   "      %%s\n" 
	   "      %%s\n"
	   "\n",
	   name_size, type_size);

  std::vector<std::string> list;

  if (sort)
    {
      for (name2option_t::iterator it = name2option.begin (); 
           it != name2option.end (); ++it)
        list.push_back (it->first);
    }
  else
    {
      list = listoptions;
    }

  for (const auto & opt_name : list)
    if (opt_name.substr (0, len) == prefix)
      {   
        auto it = name2option.find (opt_name);
	glGrib::OptionsParserDetail::optionBase * opt = it->second;
        if ((! show_hidden) && (opt->hidden))
          continue;
	char str[256];
        sprintf (&str[0], format, it->first.c_str (), opt->type ().c_str (), 
                 opt->asString ().c_str (), opt->desc.c_str ());
	help += std::string (str);
      }   

  return help;
}




const std::vector<glGrib::OptionsParser::name2option_t::iterator>
glGrib::OptionsParser::filterOptions
   (const std::string & prefix, bool show_hidden, bool sort,
    glGrib::OptionsParser * p1)
{
  std::vector<name2option_t::iterator> listOpts;

  glGrib::OptionsParser * p0 = this;

  int len = prefix.size ();

  std::vector<std::string> list;

  if (sort)
    {
      for (name2option_t::iterator it0 = p0->name2option.begin (); 
           it0 != p0->name2option.end (); ++it0)
        list.push_back (it0->first);
    }
  else
    {
      list = p0->listoptions;
    }

  for (const auto & opt_name : list)
    if (opt_name.substr (0, len) == prefix)
      {   
        auto it0 = p0->name2option.find (opt_name);
	glGrib::OptionsParserDetail::optionBase * opt0 = it0->second;

        if ((! show_hidden) && (opt0->hidden))
          continue;

        if (p1 != nullptr)
          {
            auto it1 = p1->name2option.find (it0->first);
            glGrib::OptionsParserDetail::optionBase * opt1 = it1->second;
            if (opt0->isEqual (opt1))
              continue;
          }

        listOpts.push_back (it0);
      }   

  return listOpts;
}

const std::string glGrib::OptionsParser::getJSON 
   (const std::string & prefix, bool show_hidden, bool sort,
    glGrib::OptionsParser * p1)
{
  std::string json = "[";

  for (auto & it : filterOptions (prefix, show_hidden, sort, p1))
    {
      glGrib::OptionsParserDetail::optionBase * opt = it->second;
      const std::string json_opt = opt->asJSON ();

      if (json.length () > 1)
        json += ",";

      json += std::string ("[\"") + it->first + "\",\"" 
                + opt->type () + "\",\"" + opt->desc + "\"";
 
      if (json_opt.length () > 0)
        json += std::string (",") + json_opt;
 
      json += "]";
    }

  json += "]";

  return json;
}

const std::string glGrib::OptionsBase::asOption (glGrib::OptionsBase & opt2) 
{
  auto & opt1 = *this;
  glGrib::OptionsParser p1, p2;
  opt1.traverse ("", &p1);
  opt2.traverse ("", &p2);
  return p1.asOption (p2);
}


bool glGrib::OptionsField::parseUnseen (const char * args)
{
  parse (args, &seen);
  return true;
}

namespace glGrib
{

bool operator== (const glGrib::OptionsBase & o1, const glGrib::OptionsBase & o2)
{
  glGrib::OptionsBase * _o1 = (glGrib::OptionsBase*) (&o1);
  glGrib::OptionsBase * _o2 = (glGrib::OptionsBase*) (&o2);
  glGrib::OptionsParser p1, p2;
  _o1->traverse ("", &p1);
  _o2->traverse ("", &p2);
  std::vector<std::string> str1, str2;

  p1.getValue (&str1, "--", true);
  p2.getValue (&str2, "--", true);

  return str1 == str2;
}

}
