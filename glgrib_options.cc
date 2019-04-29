#include "glgrib_options.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>


class option_base
{
public:
  option_base (const std::string & n) : name (n) {}
  virtual int has_arg () = 0;
  virtual void set (const char *) = 0;
  std::string name;
};

class option_float : public option_base
{
public:
  option_float (const std::string & n, float * v) : option_base (n), value (v)  {}
  virtual int has_arg () { return required_argument; }
  virtual void set (const char * v) { *value = std::stof (v); }
  float * value;
};

class option_string : public option_base
{
public:
  option_string (const std::string & n, std::string * v) : option_base (n), value (v)  {}
  virtual int has_arg () { return required_argument; }
  virtual void set (const char * v) { *value = std::string (v); }
  std::string * value;
};

class option_bool : public option_base
{
public:
  option_bool (const std::string & n, bool * v) : option_base (n), value (v)  {}
  virtual int has_arg () { return no_argument; }
  virtual void set (const char * v) { *value = true; }
  bool * value;
};

class option_int : public option_base
{
public:
  option_int (const std::string & n, int * v) : option_base (n), value (v)  {}
  virtual int has_arg () { return required_argument; }
  virtual void set (const char * v) { *value = std::stoi (v); }
  int * value;
};

static option_base * new_option (const std::string & n, std::string * v)
{
  return new option_string (n, v);
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
  std::map <int,option_base*> val2option;
  typedef std::vector <option_base*> optionlist;
  optionlist options;

#define ADD_OPT(x) do { options.push_back (new_option (std::string (#x), &x)); } while (0)

  ADD_OPT (field);
  ADD_OPT (field_scale);
  ADD_OPT (width);
  ADD_OPT (height);
  ADD_OPT (shell);
  ADD_OPT (geometry);
  ADD_OPT (grid);
  ADD_OPT (coasts);
  ADD_OPT (orography);
  ADD_OPT (landscape);

#undef ADD_OPT

  int nopt = options.size ();
  struct option long_options[nopt+1];

  for (int iopt = 0; iopt < nopt; iopt++)
    {
      option_base * opt = options[iopt];
      long_options[iopt] = {opt->name.c_str (), opt->has_arg (), 0, iopt};
      val2option.insert (std::pair<int,option_base *>(iopt, opt));
    }
  long_options[nopt] = {0, 0, 0, 0};
  
  int long_index = 0;
  while (1)
    {
      int iopt = getopt_long (argc, argv, "", long_options, &long_index);

      if (iopt == -1) 
        break;

      option_base * opt = val2option[iopt];

      opt->set (optarg);
    }


  for (int iopt = 0; iopt < nopt; iopt++)
    delete options[iopt];




}
