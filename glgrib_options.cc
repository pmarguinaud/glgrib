#include "glgrib_options.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>


class option_base
{
public:
  option_base (const std::string & n, const std::string & d) : name (n), desc (d) {}
  virtual int has_arg () { return 1; }
  virtual void set (const char *) = 0;
  std::string name;
  std::string desc;
  virtual std::string type () { return std::string ("UNKNOWN"); }
  virtual std::string asString () { return std::string (""); }
};

class option_float : public option_base
{
public:
  option_float (const std::string & n, const std::string & d, float * v) : option_base (n, d), value (v)  {}
  virtual void set (const char * v) 
    {
      try
        {
          *value = std::stof (v); 
	}
      catch (...)
        {
          throw std::runtime_error (std::string ("Option ") + name + std::string (" expects real values"));
	}
    }
  float * value;
  virtual std::string type () { return std::string ("FLOAT"); }
  virtual std::string asString () { std::ostringstream ss; ss << *value; return std::string (ss.str ()); }
};

class option_float_list : public option_base
{
public:
  option_float_list (const std::string & n, const std::string & d, std::vector<float> * v) : option_base (n, d), value (v)  {}
  virtual void set (const char * v) 
    {
      try
        {
          value->push_back (std::stof (v));
	}
      catch (...)
        {
          throw std::runtime_error (std::string ("Option ") + name + std::string (" expects real values"));
	}
    }
  std::vector<float> * value;
  virtual std::string type () { return std::string ("LIST OF FLOATS"); }
  virtual std::string asString () 
    { 
      std::ostringstream ss; 
      for (std::vector<float>::iterator it = value->begin(); it != value->end (); it++)
        ss << (*it) << " ";
      return std::string (ss.str ()); 
    }
};

class option_string : public option_base
{
public:
  option_string (const std::string & n, const std::string & d, std::string * v) : option_base (n, d), value (v)  {}
  virtual void set (const char * v) { *value = std::string (v); }
  std::string * value;
  virtual std::string type () { return std::string ("STRING"); }
  virtual std::string asString () { return std::string ('"' + *value + '"') ; }
};

class option_string_list : public option_base
{
public:
  option_string_list (const std::string & n, const std::string & d, std::vector<std::string> * v) : option_base (n, d), value (v)  {}
  virtual void set (const char * v) { value->push_back (std::string (v)); }
  std::vector<std::string> * value;
  virtual std::string type () { return std::string ("LIST OF STRINGS"); }
  virtual std::string asString () 
    { 
      std::ostringstream ss; 
      for (std::vector<std::string>::iterator it = value->begin(); it != value->end (); it++)
        ss << '"' + (*it) + '"' << " ";
      return std::string (ss.str ()); 
    }
};

class option_bool : public option_base
{
public:
  option_bool (const std::string & n, const std::string & d, bool * v) : option_base (n, d), value (v)  {}
  virtual int has_arg () { return 0; }
  virtual void set (const char * v) 
    { 
      if (v != NULL) 
        { 
          throw std::runtime_error (std::string ("Option ") + name + std::string (" does not take any value")); 
	} 
      *value = true; 
    }
  bool * value;
  virtual std::string type () { return std::string ("BOOLEAN"); }
  virtual std::string asString () { return std::string (""); }
};

class option_int : public option_base
{
public:
  option_int (const std::string & n, const std::string & d, int * v) : option_base (n, d), value (v)  {}
  virtual void set (const char * v) 
    { 
      try
        {
          *value = std::stoi (v); 
        }
      catch (...)
        {
          throw std::runtime_error (std::string ("Option ") + name + std::string (" expects real values"));
	}
     }
  int * value;
  virtual std::string type () { return std::string ("INTEGER"); }
  virtual std::string asString () { std::ostringstream ss; ss << *value; return std::string (ss.str ()); }
};



static option_base * new_option (const std::string & n, const std::string & d, std::vector<std::string> * v)
{
  return new option_string_list (n, d, v);
}

static option_base * new_option (const std::string & n, const std::string & d, std::string * v)
{
  return new option_string (n, d, v);
}

static option_base * new_option (const std::string & n, const std::string & d, std::vector<float> * v)
{
  return new option_float_list (n, d, v);
}

static option_base * new_option (const std::string & n, const std::string & d, float * v)
{
  return new option_float (n, d, v);
}

static option_base * new_option (const std::string & n, const std::string & d, int * v)
{
  return new option_int (n, d, v);
}

static option_base * new_option (const std::string & n, const std::string & d, bool * v)
{
  return new option_bool (n, d, v);
}

typedef std::vector <option_base*> optionlist;

static optionlist get_optionlist (glgrib_options * opts)
{
  optionlist options;
#define ADD_OPT(x,d) do { options.push_back (new_option (std::string (#x), std::string (#d), &opts->x)); } while (0)

  ADD_OPT (scene.light.on,            Enable light);
  ADD_OPT (scene.light.lon,           Light longitude);
  ADD_OPT (scene.light.lat,           Light latitude);
  ADD_OPT (scene.light.rotate,        Make sunlight move);
  ADD_OPT (scene.movie,               Movie);
  ADD_OPT (scene.movie_wait,          Wait between movie frames);
  ADD_OPT (scene.rotate_earth,        Make earth rotate);
  ADD_OPT (camera.lon,                Camera longitude);
  ADD_OPT (camera.lat,                Camera latitude);
  ADD_OPT (camera.fov,                Camera field of view);
  ADD_OPT (camera.distance,           Camera distance);

  ADD_OPT (font.bitmap,               Bitmap path);
  ADD_OPT (font.scale,                Bitmap scale);
  ADD_OPT (font.r,                    Red color);
  ADD_OPT (font.g,                    Green color);
  ADD_OPT (font.b,                    Blue color);

#define ADD_FIELD_OPT(i) \
  do {                                                                             \
  ADD_OPT (field[i].path,                List of GRIB files);                      \
  ADD_OPT (field[i].scale,               Scales to be applied to fields);          \
  ADD_OPT (field[i].palette,             Palettes);                                \
  ADD_OPT (field[i].no_value_pointer,    Do not keep field values in memory);      \
  } while (0)

  ADD_FIELD_OPT (0);
  ADD_FIELD_OPT (1);
  ADD_FIELD_OPT (2);
  ADD_FIELD_OPT (3);
  ADD_FIELD_OPT (4);
  ADD_FIELD_OPT (5);
  ADD_FIELD_OPT (6);
  ADD_FIELD_OPT (7);
  ADD_FIELD_OPT (8);
  ADD_FIELD_OPT (9);

#undef ADD_FIELD_OPT

  ADD_OPT (palette.directory,         Directory where palettes are stored);    
  ADD_OPT (colorbar.on,               Activate colorbar);
  ADD_OPT (colorbar.font.bitmap,      Bitmap path);
  ADD_OPT (colorbar.font.scale,       Bitmap scale);
  ADD_OPT (colorbar.font.r,           Red color);
  ADD_OPT (colorbar.font.g,           Green color);
  ADD_OPT (colorbar.font.b,           Blue color);

  ADD_OPT (window.width,              Window width);
  ADD_OPT (window.height,             Window height);
  ADD_OPT (window.offscreen,          Run in offline mode);
  ADD_OPT (window.offscreen_frames,   Number of frames to issue in offline mode);
  ADD_OPT (window.statistics,         Issue statistics when window is closed);
  ADD_OPT (window.title,              Window title);
  ADD_OPT (window.debug,              Enable OpenGL debugging);


  ADD_OPT (grid.resolution,           Grid resolution);
  ADD_OPT (grid.r,                    Grid red color component);
  ADD_OPT (grid.g,                    Grid green color component);
  ADD_OPT (grid.b,                    Grid blue color component);

  ADD_OPT (coastlines.path,           Path to coastlines file);
  ADD_OPT (coastlines.r,              Coastlines red color component);
  ADD_OPT (coastlines.g,              Coastlines green color component);
  ADD_OPT (coastlines.b,              Coastlines blue color component);

  ADD_OPT (landscape.orography,       Factor to apply to orography);
  ADD_OPT (landscape.path,            Path to landscape image in BMP format);
  ADD_OPT (landscape.geometry,        GRIB files to take geometry from);

  ADD_OPT (shell,                     Run command line);
  ADD_OPT (help,                      Show help message);

#undef ADD_OPT

  return options;
}

void glgrib_options::show_help ()
{
  glgrib_options opts;
  optionlist options = get_optionlist (&opts);
  printf ("Usage:\n");
  size_t name_size = 0, type_size = 0;
  for (optionlist::iterator it = options.begin (); it != options.end (); it++)
    {
      name_size = std::max ((*it)->name.length (), name_size);
      type_size = std::max ((*it)->type ().length (), type_size);
    }
  char format[64];
  sprintf (format, " --%%-%lds : %%-%lds :", name_size, type_size);
  for (optionlist::iterator it = options.begin (); it != options.end (); it++)
    {
      printf (format, (*it)->name.c_str (), (*it)->type ().c_str ());
      printf ("      %s\n", (*it)->asString ().c_str ());
      printf ("      %s\n", (*it)->desc.c_str ());
      printf ("\n");
    }
}

void glgrib_options::parse (int argc, char * argv[])
{
  std::map <std::string,option_base*> name2option;
  optionlist options = get_optionlist (this);

  // Parse options
  int nopt = options.size ();

  for (int iopt = 0; iopt < nopt; iopt++)
    {
      option_base * opt = options[iopt];
      std::string opt_name = std::string ("--") + opt->name;
      name2option.insert (std::pair<std::string,option_base *>(opt_name, opt));
      while (1)
        {
          size_t pos = opt_name.find ("_");
          if (pos == std::string::npos)
            break;
          opt_name.replace (pos, 1, "-");
        }
      name2option.insert (std::pair<std::string,option_base *>(opt_name, opt));
    }

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
      exit (EXIT_FAILURE);
    }

  if (help)
    {
      show_help ();
      exit (EXIT_SUCCESS);
    }


  for (int iopt = 0; iopt < nopt; iopt++)
    delete options[iopt];


  for (int j = 0; j < field.size (); j++)
    {
      if (field[j].scale.size () == 0)
        field[j].scale.push_back (1.00);
     
      for (int i = field[j].scale.size (); i < field[j].path.size (); i++)
        field[j].scale.push_back (field[j].scale[i-1] - 0.05);
     
      for (int i = field[j].palette.size (); i < field[j].path.size (); i++)
        field[j].palette.push_back ("default");

    }

}
