#include "glgrib_options.h"

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
  printf ("Usage:\n");
  size_t name_size = 0, type_size = 0;
  for (name2option_t::iterator it = name2option.begin (); 
       it != name2option.end (); it++)
    {   
      name_size = std::max (it->first.length (), name_size);
      type_size = std::max (it->second->type ().length (), type_size);
    }   
  char format[64];
  sprintf (format, " --%%-%lds : %%-%lds :", name_size, type_size);
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

