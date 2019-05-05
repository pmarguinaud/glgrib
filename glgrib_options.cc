#include "glgrib_options.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>


class option_base
{
public:
  option_base (const std::string & n) : name (n) {}
  virtual int has_arg () { return 1; }
  virtual void set (const char *) = 0;
  std::string name;
};

class option_float : public option_base
{
public:
  option_float (const std::string & n, float * v) : option_base (n), value (v)  {}
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
};

class option_float_list : public option_base
{
public:
  option_float_list (const std::string & n, std::vector<float> * v) : option_base (n), value (v)  {}
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
};

class option_string : public option_base
{
public:
  option_string (const std::string & n, std::string * v) : option_base (n), value (v)  {}
  virtual void set (const char * v) { *value = std::string (v); }
  std::string * value;
};

class option_string_list : public option_base
{
public:
  option_string_list (const std::string & n, std::vector<std::string> * v) : option_base (n), value (v)  {}
  virtual void set (const char * v) { value->push_back (std::string (v)); }
  std::vector<std::string> * value;
};

class option_bool : public option_base
{
public:
  option_bool (const std::string & n, bool * v) : option_base (n), value (v)  {}
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
};

class option_int : public option_base
{
public:
  option_int (const std::string & n, int * v) : option_base (n), value (v)  {}
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
};



static option_base * new_option (const std::string & n, std::vector<std::string> * v)
{
  return new option_string_list (n, v);
}

static option_base * new_option (const std::string & n, std::string * v)
{
  return new option_string (n, v);
}

static option_base * new_option (const std::string & n, std::vector<float> * v)
{
  return new option_float_list (n, v);
}

static option_base * new_option (const std::string & n, float * v)
{
  return new option_float (n, v);
}

static option_base * new_option (const std::string & n, int * v)
{
  return new option_int (n, v);
}

static option_base * new_option (const std::string & n, bool * v)
{
  return new option_bool (n, v);
}

void glgrib_options::parse (int argc, char * argv[])
{
  std::map <std::string,option_base*> name2option;
  typedef std::vector <option_base*> optionlist;
  optionlist options;


#define ADD_OPT(x) do { options.push_back (new_option (std::string (#x), &x)); } while (0)

  ADD_OPT (field.list);
  ADD_OPT (field.scale);
  ADD_OPT (field.palette);

  ADD_OPT (window.width);
  ADD_OPT (window.height);
  ADD_OPT (window.offscreen);

  ADD_OPT (shell);


  ADD_OPT (grid.resolution);

  ADD_OPT (coastlines.path);

  ADD_OPT (landscape.orography);
  ADD_OPT (landscape.path);
  ADD_OPT (landscape.geometry);


#undef ADD_OPT

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
      exit (EXIT_FAILURE);
    }



  for (int iopt = 0; iopt < nopt; iopt++)
    delete options[iopt];


  if (field.scale.size () == 0)
    field.scale.push_back (1.00);

  for (int i = field.scale.size (); i < field.list.size (); i++)
    field.scale.push_back (field.scale[i-1] - 0.05);

  for (int i = field.palette.size (); i < field.list.size (); i++)
    field.palette.push_back ("default");

}
