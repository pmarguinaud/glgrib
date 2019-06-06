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



#include <string>
#include <vector>

class glgrib_option_callback
{
public:
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, float * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, bool * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, int * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<std::string> * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::string * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<float> * data) {}
};

class glgrib_option_parser : public glgrib_option_callback
{
public:
  ~glgrib_option_parser ()
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); it++)
      delete it->second;
  }

private:

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

  class name2option_t : public std::map<std::string,option_base*> 
  {
  public:
    void insert (const std::string name, option_base * opt)
    {
      std::map<std::string,option_base*>::insert (std::pair<std::string,option_base *>(name, opt));
    }
  };

  name2option_t name2option;


  std::string get_opt_name (const std::string & path, const std::string & name)
  {
    return "--" + path + (path == "" ? "" : ".") + name;
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, float * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_float (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, bool * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_bool (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, int * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_int (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<std::string> * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_string_list (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::string * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_string (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<float> * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_float_list (opt_name, desc, data));
  }


public:

  void dump ()
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); it++)
      {
        std::cout << it->first << std::endl;
      }
  }

  void parse (int argc, char * argv[])
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
  }

  void show_help ()
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
};

class glgrib_option_base 
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<float> float_list;
  typedef std::string string;
  virtual void traverse (const std::string &, glgrib_option_callback *) {}
};


#define APPLY(name, desc) do { cb->apply (p, #name, #desc, &name); } while (0)
#define TRAVERSE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb); } while (0)

class glgrib_option_vector : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (on,          Field is a vector);          
    APPLY (hide_arrow,  Hide arrows);                
    APPLY (hide_norm,   Hide norm field);            
    APPLY (r,           Red color for arrows);       
    APPLY (g,           Green color for arrows);     
    APPLY (b,           Blue color for arrows);      
  }
  bool  on         = false;
  bool  hide_arrow = false;
  bool  hide_norm  = false;
  int   r  = 255;
  int   g  = 255;
  int   b  = 255;
};

class glgrib_option_field : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (path,             List of GRIB files);                    
    APPLY (scale,            Scales to be applied to fields);        
    APPLY (palette,          Palettes);                              
    APPLY (no_value_pointer, Do not keep field values in memory);    
    TRAVERSE (vector);
  }
  string_list  path;
  string_list  palette;
  float_list   scale;
  bool         no_value_pointer = false;
  glgrib_option_vector vector;
};


class glgrib_option_palette : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (directory,        Directory where palettes are stored);    
  }
  string  directory  = "palettes";
};

class glgrib_option_grid : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (resolution,        Grid resolution);
    APPLY (r,                 Grid red color component);
    APPLY (g,                 Grid green color component);
    APPLY (b,                 Grid blue color component);
  }
  int resolution = 9;
  int r = 255; 
  int g = 255; 
  int b = 255;
};

class glgrib_option_landscape : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (orography,           Factor to apply to orography);
    APPLY (path,                Path to landscape image in BMP format);
    APPLY (geometry,            GRIB files to take geometry from);
    APPLY (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
  }
  string  path  = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  float  orography  = 0.05;
  string  geometry  = "";
  int  number_of_latitudes  = 500;
};

class glgrib_option_coastlines : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (path,               Path to coastlines file);
    APPLY (r,                  Coastlines red color component);
    APPLY (g,                  Coastlines green color component);
    APPLY (b,                  Coastlines blue color component);
  }
  string path  = "coastlines/gshhs(3).rim";
  int r = 255;
  int g = 255;
  int b = 255;
};

class glgrib_option_window : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (width,              Window width);
    APPLY (height,             Window height);
    APPLY (offscreen,          Run in offline mode);
    APPLY (offscreen_frames,   Number of frames to issue in offline mode);
    APPLY (statistics,         Issue statistics when window is closed);
    APPLY (title,              Window title);
    APPLY (debug,              Enable OpenGL debugging);
  }
  int     width  = 800;
  int     height  = 800;
  bool    offscreen  = false;
  int     offscreen_frames  = 1;
  bool    statistics  = false;
  string  title  = "";
  bool    debug  = false;
};

class glgrib_option_light : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (on,            Enable light);
    APPLY (lon,           Light longitude);
    APPLY (lat,           Light latitude);
    APPLY (rotate,        Make sunlight move);
  }
  bool   on  = false;
  bool   rotate  = false;
  float  lon  = 0.0f;
  float  lat  = 0.0f;
};

class glgrib_option_scene : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (movie,               Movie);
    APPLY (movie_wait,          Wait between movie frames);
    APPLY (rotate_earth,        Make earth rotate);
    APPLY (projection,          Mercator XYZ latlon polar_north polar_south);
    APPLY (transformation,      Perspective or orthographic);
    TRAVERSE (light);
  }
  bool    rotate_earth  = false;
  bool    movie  = false;
  float   movie_wait  = 1.0f;
  string  projection  = "XYZ";
  string  transformation  = "PERSPECTIVE";
  glgrib_option_light   light  ;
};

class glgrib_option_camera : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (lon,                Camera longitude);
    APPLY (lat,                Camera latitude);
    APPLY (fov,                Camera field of view);
    APPLY (distance,           Camera distance);
  }
  float  distance  = 6.0; 
  float  lat       = 0.0; 
  float  lon       = 0.0; 
  float  fov       = 20.;
};

class glgrib_option_font : public glgrib_option_base
{
public:
  glgrib_option_font (const std::string & b, float s) : bitmap (b), scale (s) {}
  glgrib_option_font () {}
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (bitmap, Bitmap path);
    APPLY (scale,  Bitmap scale);
    APPLY (r,      Red color);
    APPLY (g,      Green color);
    APPLY (b,      Blue color);
  }
  std::string bitmap = "fonts/08.bmp";
  float scale = 0.05f;
  int r = 255, g = 255, b = 255;
};


class glgrib_option_colorbar : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    APPLY (on, Activate colorbar);
    TRAVERSE (font);
  }
  bool on = false;
  glgrib_option_font font = glgrib_option_font ("fonts/16.bmp", 0.02f);
};

class glgrib_option : public glgrib_option_base
{
public:
  virtual void traverse (const std::string & p, glgrib_option_callback * cb)
  {
    TRAVERSE (field[0]); TRAVERSE (field[1]); TRAVERSE (field[2]); TRAVERSE (field[3]); TRAVERSE (field[4]); 
    TRAVERSE (field[5]); TRAVERSE (field[6]); TRAVERSE (field[7]); TRAVERSE (field[8]); TRAVERSE (field[9]); 
    TRAVERSE (palette);
    TRAVERSE (coastlines);
    TRAVERSE (window);
    TRAVERSE (landscape);
    TRAVERSE (grid);
    TRAVERSE (scene);
    TRAVERSE (camera);
    TRAVERSE (colorbar);
    TRAVERSE (font);
    APPLY (shell, Run command line);
    APPLY (help, Show help message);
  }
  std::vector<glgrib_option_field> field =
    {glgrib_option_field (), glgrib_option_field (), glgrib_option_field (), glgrib_option_field (), glgrib_option_field (), 
     glgrib_option_field (), glgrib_option_field (), glgrib_option_field (), glgrib_option_field (), glgrib_option_field ()};
  glgrib_option_palette palette;
  glgrib_option_coastlines coastlines;
  glgrib_option_colorbar colorbar;
  glgrib_option_window window;
  glgrib_option_landscape landscape;
  glgrib_option_grid grid;
  glgrib_option_scene scene;
  glgrib_option_camera camera;
  glgrib_option_font font;
  bool shell = false;
  bool help = false;
};


int main (int argc, char * argv[])
{
  glgrib_option opts;
  glgrib_option_parser oh;
  opts.traverse ("", &oh);
  oh.parse (argc, argv);
  return 0;
}
