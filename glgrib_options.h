#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <time.h>

class glgrib_option_color
{
public:
  static glgrib_option_color color_by_name (const char *);
  static glgrib_option_color color_by_hexa (const char *);
  static void parse (int *, glgrib_option_color *, const char *);

  glgrib_option_color () {}
  glgrib_option_color (int _r, int _g, int _b, int _a = 255) : r (_r), g (_g), b (_b), a (_a) {}
  int r = 255, g = 255, b = 255, a = 255;
};

class glgrib_option_date
{
public:
  glgrib_option_date () {}
  glgrib_option_date (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  static glgrib_option_date interpolate (const glgrib_option_date &, const glgrib_option_date &, const float);
  static glgrib_option_date date_from_t (time_t);
  static time_t t_from_date (const glgrib_option_date &);
  std::string asString () const;
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
            glgrib_option_color::parse (&count, value, v);
          }
        catch (...)
          {
            throw std::runtime_error (std::string ("Option ") + name + std::string (" expects integer values or color names, or hexadecimal codes"));
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
            if (count == 0)
              value->push_back (glgrib_option_color ());
            int last = value->size () - 1;

            glgrib_option_color::parse (&count, &(*value)[last], v);
            
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


#define DESC(name, desc) do { cb->apply (p, #name, #desc, &name); } while (0)
#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb); } while (0)
#define DEFINE virtual void traverse (const std::string & p, glgrib_options_callback * cb)

class glgrib_options_contour : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,            Enable contour);          
    DESC (colors,        Contour colors);
    DESC (number,        Number of levels);
    DESC (levels,        List of levels);
    DESC (widths,        List of widths);
    DESC (patterns,      List of dash patterns);
    DESC (lengths,       List of dash lengths);
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
  DEFINE
  {
    DESC (on,          Field is a vector);          
    DESC (hide_arrow,  Hide arrows);                
    DESC (hide_norm,   Hide norm field);            
    DESC (color,       Color for arrows);
    DESC (density,     Vector density);
    DESC (scale,       Vector scale);
    DESC (head_size,   Vector head size);
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
  DEFINE
  {
    DESC (path,             List of GRIB files);                    
    DESC (scale,            Scales to be applied to fields);        
    DESC (palette,          Palettes);                              
    DESC (no_value_pointer, Do not keep field values in memory);    
    DESC (diff,             Show field difference);
    INCLUDE (vector);
    INCLUDE (contour);
  }
  string_list  path;
  string       palette = "default";
  float        scale   = 1.0f;
  bool         no_value_pointer = false;
  bool         diff = false;
  glgrib_options_vector vector;
  glgrib_options_contour contour;
};


class glgrib_options_palette : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (directory,        Directory where palettes are stored);    
  }
  string  directory  = "palettes";
};

class glgrib_options_grid : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,                Display grid);
    DESC (resolution,        Grid resolution);
    DESC (color,             Grid color);
    DESC (scale,             Grid scale);
  }
  int resolution = 9;
  glgrib_option_color color = glgrib_option_color (0, 255, 0);
  bool on = false;
  float scale = 1.005;
};

class glgrib_options_landscape : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,                  Enable landscape);
    DESC (orography,           Factor to apply to orography);
    DESC (path,                Path to landscape image in BMP format);
    DESC (geometry,            GRIB files to take geometry from);
    DESC (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
    DESC (wireframe,           Draw landscape in wireframe mode);
  }
  string  path  = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  float  orography  = 0.05;
  string  geometry  = "";
  int  number_of_latitudes  = 500;
  bool wireframe = false;
  bool on = false;
};

class glgrib_options_coastlines : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,                 Display coastlines);
    DESC (path,               Path to coastlines file);
    DESC (color,              Coastlines color);
    DESC (scale,              Coastlines scale);
  }
  string path  = "coastlines/gshhs(3).rim";
  glgrib_option_color color;
  bool on = false;
  float scale = 1.005;
};

class glgrib_options_offscreen : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,       Run in offline mode);
    DESC (frames,   Number of frames to issue in offline mode);
    DESC (format,   PNG format output);
  }
  bool on  = false;
  int  frames  = 1;
  std::string format = "snapshot_%N.png";
};

class glgrib_options_window : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (width,              Window width);
    DESC (height,             Window height);
    DESC (statistics,         Issue statistics when window is closed);
    DESC (title,              Window title);
    DESC (debug,              Enable OpenGL debugging);
    DESC (version_major,      GLFW_CONTEXT_VERSION_MAJOR);
    DESC (version_minor,      GLFW_CONTEXT_VERSION_MINOR);
    INCLUDE (offscreen);
  }
  int     width  = 800;
  int     height  = 800;
  bool    statistics  = false;
  string  title  = "";
  bool    debug  = false;
  int     version_major = 4;
  int     version_minor = 3;
  glgrib_options_offscreen offscreen;
};

class glgrib_options_light : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (date_from_grib, Calculate light position from GRIB date);
    DESC (on,             Enable light);
    DESC (lon,            Light longitude);
    DESC (lat,            Light latitude);
    DESC (rotate,         Make sunlight move);
    DESC (date,           Date for sunlight position);
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
  DEFINE
  {
    DESC (lon,            Longitude);
    DESC (lat,            Latitude);
    DESC (fov,            Field of view);
  }
  float  lon  = 0.0f;
  float  lat  = 0.0f;
  float  fov  = 0.0f;
};

class glgrib_options_travelling : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,             Enable travelling); 
    DESC (frames,         Number of frames);
    INCLUDE (pos1);
    INCLUDE (pos2);
  }
  bool   on     = false;
  int    frames = 100;
  glgrib_options_position pos1;
  glgrib_options_position pos2;
};

class glgrib_options_interpolation : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,             Enable interpolation);
    DESC (frames,         Number of frames);
  }
  bool on = false;
  int frames = 10;
};

class glgrib_options_image : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,            Enable image display);
    DESC (path,          Path to image);
    DESC (x0,            Left abscissa);
    DESC (y0,            Lower ordinate);
    DESC (x1,            Right abscissa);
    DESC (y1,            Upper ordinate);
    DESC (align,         Image alignment);
  }  
  bool on = false;
  std::string path = "";
  float x0 = 0.0, x1 = 1.0, y0 = 0.0, y1 = 1.0;
  std::string align;
};

class glgrib_options_scene : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (lon_at_hour,         Set longitude at solar time);
    DESC (rotate_earth,        Make earth rotate);
    DESC (projection,          Mercator XYZ latlon polar_north polar_south);
    DESC (transformation,      Perspective or orthographic);
    INCLUDE (light);
    INCLUDE (travelling);
    DESC (test_strxyz,         Test XYZ string);
    INCLUDE (interpolation);
    DESC (display_date,        Display date);
    DESC (text,                Strings to be displayed);
    DESC (text_x,              Coordinates of strings);
    DESC (text_y,              Coordinates of strings);
    DESC (text_a,              Text alignment);
    INCLUDE (image);
  }
  bool    rotate_earth  = false;
  float   lon_at_hour = -1.0f;
  string  projection  = "XYZ";
  string  transformation  = "PERSPECTIVE";
  glgrib_options_light light;  
  glgrib_options_travelling travelling;
  bool    test_strxyz = false;
  glgrib_options_interpolation interpolation;
  bool    display_date = false;
  std::vector<std::string> text;
  std::vector<float> text_x;
  std::vector<float> text_y;
  std::vector<std::string> text_a;
  glgrib_options_image image;
};

class glgrib_options_camera : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (lon,                Camera longitude);
    DESC (lat,                Camera latitude);
    DESC (fov,                Camera field of view);
    DESC (distance,           Camera distance);
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
  DEFINE
  {
    DESC (bitmap,     Bitmap path);
    DESC (scale,      Bitmap scale);
    DESC (color.foreground, Foreground color);
    DESC (color.background, Background color);
  }
  std::string bitmap = "fonts/08.bmp";
  float scale = 0.05f;
  struct
  {
    glgrib_option_color foreground = glgrib_option_color (255, 255, 255, 255);
    glgrib_option_color background = glgrib_option_color (  0,   0,   0,   0);
  } color;
};


class glgrib_options_colorbar : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Activate colorbar);
    INCLUDE (font);
    DESC (format, "Format (sprintf) use to display numbers");
  }
  bool on = false;
  glgrib_options_font font = glgrib_options_font ("fonts/16.bmp", 0.02f);
  std::string format = "%6.4g";
  int levels = 11;
};

class glgrib_options : public glgrib_options_base
{
public:
  DEFINE
  {
    INCLUDE (field[0]); INCLUDE (field[1]); INCLUDE (field[2]); INCLUDE (field[3]); INCLUDE (field[4]); 
    INCLUDE (field[5]); INCLUDE (field[6]); INCLUDE (field[7]); INCLUDE (field[8]); INCLUDE (field[9]); 
    INCLUDE (palette);
    INCLUDE (coastlines);
    INCLUDE (window);
    INCLUDE (landscape);
    INCLUDE (grid);
    INCLUDE (scene);
    INCLUDE (camera);
    INCLUDE (colorbar);
    INCLUDE (font);
    DESC (shell, Run command line);
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

