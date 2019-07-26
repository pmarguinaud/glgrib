#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

class glgrib_option_color
{
public:
  glgrib_option_color () {}
  glgrib_option_color (int _r, int _g, int _b) : r (_r), g (_g), b (_b) {}
  int r = 255, g = 255, b = 255;
};

class glgrib_option_date
{
public:
  glgrib_option_date () {}
  glgrib_option_date (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
};


class glgrib_options_callback
{
public:
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, float * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, bool * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, int * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<std::string> * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::string * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<float> * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<int> * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, glgrib_option_color * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<glgrib_option_color> * data) {}
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, glgrib_option_date * data) {}
};

class glgrib_options_parser : public glgrib_options_callback
{
public:
  bool parse (int, char * []);
  void show_help ();
  ~glgrib_options_parser ()
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
  class option_int_list : public option_base
  {
  public:
    option_int_list (const std::string & n, const std::string & d, std::vector<int> * v) : option_base (n, d), value (v)  {}  
    virtual void set (const char * v)  
      {   
        try 
          {
            value->push_back (std::stoi (v));
          }
        catch (...)
          {
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values"));
          }
      }   
    std::vector<int> * value;
    virtual std::string type () { return std::string ("LIST OF INTEGERS"); }
    virtual std::string asString ()
      {
        std::ostringstream ss;
        for (std::vector<int>::iterator it = value->begin(); it != value->end (); it++)
          ss << (*it) << " ";
        return std::string (ss.str ());
      }
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
  class option_date : public option_base
  {
  public:
    option_date (const std::string & n, const std::string & d, glgrib_option_date * v) : option_base (n, d), value (v)  {}  
    virtual void set (const char * v)  
      {   
        try 
          {
            int c = std::stoi (v);
            switch (count)
              {
                case 0: value->year   = c; break;
                case 1: value->month  = c; break;
                case 2: value->day    = c; break;
                case 3: value->hour   = c; break;
                case 4: value->minute = c; break;
                case 5: value->second = c; break;
              }
            count++;
          }
        catch (...)
          {
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values"));
          }
      }   
    glgrib_option_date * value;
    virtual std::string type () { return std::string ("DATE YEAR MONTH DAY HOUR MINUTE SECOND"); }
    virtual std::string asString ()
      {
        std::ostringstream ss;
        ss << value->year << " " << value->month << " " << value->day << " " << value->hour << " " << value->minute << " " << value->second;
        return std::string (ss.str ());
      }
  private:
    int count = 0;
  };
  class option_color : public option_base
  {
  public:
    option_color (const std::string & n, const std::string & d, glgrib_option_color * v) : option_base (n, d), value (v)  {}  
    virtual void set (const char * v)  
      {   
        try 
          {
            int c = std::stoi (v);
            switch (count)
              {
                case 0: value->r = c; break;
                case 1: value->g = c; break;
                case 2: value->b = c; break;
              }
            count++;
          }
        catch (...)
          {
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values"));
          }
      }   
    glgrib_option_color * value;
    virtual std::string type () { return std::string ("COLOR R G B"); }
    virtual std::string asString ()
      {
        std::ostringstream ss;
        ss << value->r << " " << value->g << " " << value->b;
        return std::string (ss.str ());
      }
  private:
    int count = 0;
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
  class option_color_list : public option_base
  {
  public:
    option_color_list (const std::string & n, const std::string & d, std::vector<glgrib_option_color> * v) : option_base (n, d), value (v)  {}
    virtual void set (const char * v) 
      { 
        try 
          {
            int c = std::stoi (v);
            if (count == 0)
              value->push_back (glgrib_option_color ());
            int last = value->size () - 1;
            switch (count)
              {
                case 0: (*value)[last].r = c; break;
                case 1: (*value)[last].g = c; break;
                case 2: (*value)[last].b = c; break;
              }
            count++;
            count = count % 3;
          }
        catch (...)
          {
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values"));
          }
      }
    std::vector<glgrib_option_color> * value;
    virtual std::string type () { return std::string ("LIST OF COLORS R G B"); }
    virtual std::string asString ()
      {
        std::ostringstream ss;
        for (std::vector<glgrib_option_color>::iterator it = value->begin(); it != value->end (); it++)
          ss << it->r << " " << it->g << " " << it->b;
        return std::string (ss.str ());
      }
  private:
    int count = 0;
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
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values"));
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
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<int> * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_int_list (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, glgrib_option_color * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_color (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, std::vector<glgrib_option_color> * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_color_list (opt_name, desc, data));
  }
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, glgrib_option_date * data) 
  {
    std::string opt_name = get_opt_name (path, name);
    name2option.insert (opt_name, new option_date (opt_name, desc, data));
  }


};

class glgrib_options_base 
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<float> float_list;
  typedef std::string string;
  virtual void traverse (const std::string &, glgrib_options_callback *) {}
  virtual bool parse (int argc, char * argv[])
  {
    glgrib_options_parser p;
    traverse ("", &p);
    return p.parse (argc, argv);
  }
};


#define APPLY(name, desc) do { cb->apply (p, #name, #desc, &name); } while (0)
#define TRAVERSE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb); } while (0)
#define TRAVERSE_DEF virtual void traverse (const std::string & p, glgrib_options_callback * cb)

class glgrib_options_contour : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (on,            Enable contour);          
    APPLY (colors,        Contour colors);
    APPLY (number,        Number of levels);
    APPLY (levels,        List of levels);
    APPLY (widths,        List of widths);
    APPLY (patterns,      List of dash patterns);
    APPLY (lengths,       List of dash lengths);
  }
  bool on = false;
  int number = 10;
  std::vector<float> levels;
  std::vector<float> widths;
  std::vector<glgrib_option_color> colors;
  std::vector<std::string> patterns;
  std::vector<float> lengths;
};

class glgrib_options_vector : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (on,          Field is a vector);          
    APPLY (hide_arrow,  Hide arrows);                
    APPLY (hide_norm,   Hide norm field);            
    APPLY (color,       Color for arrows);
    APPLY (density,     Vector density);
    APPLY (scale,       Vector scale);
    APPLY (head_size,   Vector head size);
  }
  bool  on         = false;
  bool  hide_arrow = false;
  bool  hide_norm  = false;
  glgrib_option_color color;
  float density = 50.0f;
  float scale = 1.0f;
  float head_size = 0.1f;
};

class glgrib_options_field : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (path,             List of GRIB files);                    
    APPLY (scale,            Scales to be applied to fields);        
    APPLY (palette,          Palettes);                              
    APPLY (no_value_pointer, Do not keep field values in memory);    
    TRAVERSE (vector);
    TRAVERSE (contour);
  }
  string_list  path;
  string_list  palette;
  float_list   scale;
  bool         no_value_pointer = false;
  glgrib_options_vector vector;
  glgrib_options_contour contour;
};


class glgrib_options_palette : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (directory,        Directory where palettes are stored);    
  }
  string  directory  = "palettes";
};

class glgrib_options_grid : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (resolution,        Grid resolution);
    APPLY (color,             Grid color);
  }
  int resolution = 9;
  glgrib_option_color color = glgrib_option_color (0, 255, 0);
};

class glgrib_options_landscape : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (orography,           Factor to apply to orography);
    APPLY (path,                Path to landscape image in BMP format);
    APPLY (geometry,            GRIB files to take geometry from);
    APPLY (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
    APPLY (wireframe,           Draw landscape in wireframe mode);
  }
  string  path  = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  float  orography  = 0.05;
  string  geometry  = "";
  int  number_of_latitudes  = 500;
  bool wireframe = false;
};

class glgrib_options_coastlines : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (path,               Path to coastlines file);
    APPLY (color,              Coastlines color);
  }
  string path  = "coastlines/gshhs(3).rim";
  glgrib_option_color color;
};

class glgrib_options_window : public glgrib_options_base
{
public:
  TRAVERSE_DEF
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

class glgrib_options_light : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (date_from_grib, Calculate light position from GRIB date);
    APPLY (on,             Enable light);
    APPLY (lon,            Light longitude);
    APPLY (lat,            Light latitude);
    APPLY (rotate,         Make sunlight move);
    APPLY (date,           Date for sunlight position);
  }
  glgrib_option_date date;
  bool   date_from_grib = false;
  bool   on  = false;
  bool   rotate  = false;
  float  lon  = 0.0f;
  float  lat  = 0.0f;
};

class glgrib_options_position : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (lon,            Longitude);
    APPLY (lat,            Latitude);
    APPLY (fov,            Field of view);
  }
  float  lon  = 0.0f;
  float  lat  = 0.0f;
  float  fov  = 0.0f;
};

class glgrib_options_travelling : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (on,             Enable travelling); 
    APPLY (frames,         Number of frames);
    TRAVERSE (pos1);
    TRAVERSE (pos2);
  }
  bool   on     = false;
  int    frames = 100;
  glgrib_options_position pos1;
  glgrib_options_position pos2;
};

class glgrib_options_scene : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (movie,               Movie);
    APPLY (movie_wait,          Wait between movie frames);
    APPLY (rotate_earth,        Make earth rotate);
    APPLY (projection,          Mercator XYZ latlon polar_north polar_south);
    APPLY (transformation,      Perspective or orthographic);
    TRAVERSE (light);
    TRAVERSE (travelling);
    APPLY (test_strxyz,         Test XYZ string);
  }
  bool    rotate_earth  = false;
  bool    movie  = false;
  float   movie_wait  = 1.0f;
  string  projection  = "XYZ";
  string  transformation  = "PERSPECTIVE";
  glgrib_options_light light;  
  glgrib_options_travelling travelling;
  bool    test_strxyz = false;
};

class glgrib_options_camera : public glgrib_options_base
{
public:
  TRAVERSE_DEF
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

class glgrib_options_font : public glgrib_options_base
{
public:
  glgrib_options_font (const std::string & b, float s) : bitmap (b), scale (s) {}
  glgrib_options_font () {}
  TRAVERSE_DEF
  {
    APPLY (bitmap, Bitmap path);
    APPLY (scale,  Bitmap scale);
    APPLY (color,  Font color);
  }
  std::string bitmap = "fonts/08.bmp";
  float scale = 0.05f;
  glgrib_option_color color;
};


class glgrib_options_colorbar : public glgrib_options_base
{
public:
  TRAVERSE_DEF
  {
    APPLY (on, Activate colorbar);
    TRAVERSE (font);
  }
  bool on = false;
  glgrib_options_font font = glgrib_options_font ("fonts/16.bmp", 0.02f);
};

class glgrib_options : public glgrib_options_base
{
public:
  TRAVERSE_DEF
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
  }
  std::vector<glgrib_options_field> field =
    {glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field (), glgrib_options_field ()};
  glgrib_options_palette palette;
  glgrib_options_coastlines coastlines;
  glgrib_options_colorbar colorbar;
  glgrib_options_window window;
  glgrib_options_landscape landscape;
  glgrib_options_grid grid;
  glgrib_options_scene scene;
  glgrib_options_camera camera;
  glgrib_options_font font;
  bool shell = false;
  virtual bool parse (int, char * []);
};

#endif

