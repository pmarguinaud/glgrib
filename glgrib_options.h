#ifndef _GLGRIB_OPTIONS_H
#define _GLGRIB_OPTIONS_H

#include <map>
#include <set>
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
  static void parse (glgrib_option_color *, const char *);

  glgrib_option_color () {}
  glgrib_option_color (int _r, int _g, int _b, int _a = 255) : r (_r), g (_g), b (_b), a (_a) {}
  int r = 255, g = 255, b = 255, a = 255;
  std::string asString () const 
  {
    char str[32]; 
    sprintf (str, "#%2.2x%2.2x%2.2x%2.2x", r, g, b, a); 
    return std::string (str); 
  }
  friend std::ostream & operator << (std::ostream &, const glgrib_option_color &);
  friend std::istream & operator >> (std::istream &, glgrib_option_color &);
  friend bool operator== (glgrib_option_color const & col1, glgrib_option_color const & col2)
  {
    return (col1.r == col2.r)
        && (col1.g == col2.g)
        && (col1.b == col2.b)
        && (col1.a == col2.a);
  }
};

class glgrib_option_date
{
public:
  static void parse (glgrib_option_date *, const char *);
  glgrib_option_date () {}
  glgrib_option_date (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  static glgrib_option_date interpolate (const glgrib_option_date &, const glgrib_option_date &, const float);
  static glgrib_option_date date_from_t (time_t);
  static time_t t_from_date (const glgrib_option_date &);
  std::string asString () const;
  friend std::ostream & operator << (std::ostream &, const glgrib_option_date &);
  friend std::istream & operator >> (std::istream &, glgrib_option_date &);
  friend bool operator== (glgrib_option_date const & d1, glgrib_option_date const & d2)
  {
    return (d1.year   == d2.year  )
        && (d1.month  == d2.month )
        && (d1.day    == d2.day   )
        && (d1.hour   == d2.hour  )
        && (d1.minute == d2.minute)
        && (d1.second == d2.second);
  }
};



namespace glgrib_options_parser_detail 
{

  class option_base
  {
  public:
    option_base (const std::string & n, const std::string & d) : name (n), desc (d) {}
    virtual int has_arg () const { return 1; }
    virtual void set (const char *) = 0;
    std::string name;
    std::string desc;
    virtual std::string type ()  = 0;
    virtual std::string asString () const = 0;
    virtual void clear () = 0;
    virtual bool isEqual (const option_base *) const = 0;
    bool hidden = false;
  };

  template <class T>
  class option_tmpl : public option_base
  {
  public:
    option_tmpl (const std::string & n, const std::string & d, T * v = NULL) : option_base (n, d), value (v) {}
    T * value = NULL;
    std::string asString () const { std::ostringstream ss; ss << *value; return std::string (ss.str ()); }
    void set (const char * v)  
    {   
      try 
        {
          std::stringstream in (v);
          in >> *value;
        }
      catch (...)
        {
          throw std::runtime_error (std::string ("Parsing option ") + name 
                + std::string (" failed"));
        }
    }   
    std::string type () { return std::string ("UNKNOWN"); }
    void clear () {}
    int has_arg () const { return 1; }
    bool isEqual (const option_base * _o) const
    {
      const option_tmpl<T> * o = NULL;
      try
        {
          o = dynamic_cast<const option_tmpl<T>*>(_o);
	}
      catch (...)
        {
          return false;
        }
      return *(o->value) == *value;
    }
  };

  template <class T>
  class option_tmpl_list : public option_base
  {
  public:
    option_tmpl_list (const std::string & n, const std::string & d, std::vector<T> * v = NULL) : option_base (n, d), value (v) {}
    std::vector<T> * value = NULL;
    std::string asString () const
    {
      std::ostringstream ss;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); it++)
        ss << (*it) << " ";
      return std::string (ss.str ());
    }
    void set (const char * v)  
    {   
      try 
        {
          std::stringstream in (v);
          T val;
	  in >> val;
          value->push_back (val);
        }
      catch (...)
        {
          throw std::runtime_error (std::string ("Parsing option ") + name 
                + std::string (" failed"));
        }
    }   
    void clear () { if (value) value->clear (); }
    std::string type () { return std::string ("UNKNOWN"); }
    bool isEqual (const option_base * _o) const 
    {
      const option_tmpl_list<T> * o = NULL;
      try
        {
          o = dynamic_cast<const option_tmpl_list<T>*>(_o);
	}
      catch (...)
        {
          return false;
        }
      return *(o->value) == *value;
    }
  };

  template <> std::string option_tmpl     <int>                ::type ();
  template <> std::string option_tmpl     <float>              ::type ();
  template <> std::string option_tmpl_list<int>                ::type ();
  template <> std::string option_tmpl_list<float>              ::type ();
  template <> std::string option_tmpl     <glgrib_option_date> ::type ();
  template <> std::string option_tmpl     <glgrib_option_color>::type ();
  template <> std::string option_tmpl     <std::string>        ::type ();
  template <> std::string option_tmpl     <std::string>        ::asString () const;
  template <> std::string option_tmpl_list<glgrib_option_color>::type ();
  template <> std::string option_tmpl_list<std::string>        ::type ();
  template <> std::string option_tmpl_list<std::string>        ::asString () const;
  template <> std::string option_tmpl     <bool>               ::type ();
  template <> void option_tmpl<bool>::set (const char *);
  template <> void option_tmpl<bool>::clear ();
  template <> std::string option_tmpl<bool>::asString () const;
  template <> int option_tmpl<bool>::has_arg () const;

};

class glgrib_options_callback
{
public:

  class opt
  {
  public:
  virtual void _dummy_ () {}
  };

#define DEF_APPLY(T) \
  virtual void apply (const std::string & path, const std::string & name, const std::string & desc, T * data, const opt * = NULL) {}
  DEF_APPLY (float);
  DEF_APPLY (bool);
  DEF_APPLY (int);
  DEF_APPLY (std::vector<std::string>);
  DEF_APPLY (std::string);
  DEF_APPLY (std::vector<float>);
  DEF_APPLY (std::vector<int>);
  DEF_APPLY (glgrib_option_color);
  DEF_APPLY (std::vector<glgrib_option_color>);
  DEF_APPLY (glgrib_option_date);
#undef DEF_APPLY
};


class glgrib_options_parser : public glgrib_options_callback
{
public:
  static void print (class glgrib_options &);
  bool parse (int, const char * []);
  void show_help ();
  void display (const std::string &, bool = false);
  ~glgrib_options_parser ()
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); it++)
      delete it->second;
  }
  
  bool seenOption (const std::string &) const;
  void getOptions (std::vector<std::string> * vs)
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); it++)
      vs->push_back (it->first);
  }

  std::set<std::string> getSeenOptions () const
  {
    return seen;
  }

  const glgrib_options_parser_detail::option_base * getOption (const std::string & name)
  {
    name2option_t::iterator it = name2option.find (name);
    if (it != name2option.end ())
      return it->second;
    return NULL;
  }

  class opt : public glgrib_options_callback::opt
  {
  public:
    bool hidden = false;
  };

private:

  std::vector<std::string> ctx;
  std::set<std::string> seen;

  class name2option_t : public std::map<std::string,glgrib_options_parser_detail::option_base*> 
  {
  public:
    void insert (const std::string name, glgrib_options_parser_detail::option_base * opt)
    {
      std::map<std::string,glgrib_options_parser_detail::option_base*>::insert 
        (std::pair<std::string,glgrib_options_parser_detail::option_base *>(name, opt));
    }
  };

  name2option_t name2option;

  std::string get_opt_name (const std::string & path, const std::string & name)
  {
    return "--" + path + (path == "" ? "" : ".") + name;
  }

  void createOption (const std::string & opt_name, 
                     glgrib_options_parser_detail::option_base * option, 
                     const glgrib_options_callback::opt * _o)
  {
    const opt * o = dynamic_cast<const opt*>(_o);
    if (o != NULL)
      option->hidden = o->hidden;
    name2option.insert (opt_name, option);
  }

#define DEF_APPLY(T,C) \
  void apply (const std::string & path, const std::string & name,                      \
              const std::string & desc, T * data,                                      \
              const glgrib_options_callback::opt * o = NULL)                           \
  {                                                                                    \
    std::string opt_name = get_opt_name (path, name);                                  \
    createOption (opt_name, new C (opt_name, desc, data), o);                          \
  }

  DEF_APPLY (float                             , glgrib_options_parser_detail::option_tmpl<float> );
  DEF_APPLY (bool                              , glgrib_options_parser_detail::option_tmpl<bool>);
  DEF_APPLY (int                               , glgrib_options_parser_detail::option_tmpl<int>);
  DEF_APPLY (std::vector<std::string>          , glgrib_options_parser_detail::option_tmpl_list<std::string>);
  DEF_APPLY (std::string                       , glgrib_options_parser_detail::option_tmpl<std::string>);
  DEF_APPLY (std::vector<float>                , glgrib_options_parser_detail::option_tmpl_list<float>);
  DEF_APPLY (std::vector<int>                  , glgrib_options_parser_detail::option_tmpl_list<int>);
  DEF_APPLY (glgrib_option_color               , glgrib_options_parser_detail::option_tmpl<glgrib_option_color>);
  DEF_APPLY (std::vector<glgrib_option_color>  , glgrib_options_parser_detail::option_tmpl_list<glgrib_option_color>);
  DEF_APPLY (glgrib_option_date                , glgrib_options_parser_detail::option_tmpl<glgrib_option_date>);

#undef DEF_APPLY

};

class glgrib_options_base 
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<float> float_list;
  typedef std::string string;
  virtual void traverse (const std::string &, glgrib_options_callback *, 
                         const glgrib_options_callback::opt * = NULL) {}
  virtual bool parse (int argc, const char * argv[])
  {
    glgrib_options_parser p;
    traverse ("", &p);
    return p.parse (argc, argv);
  }
};


#define DESC(name, desc) do { cb->apply (p, #name, #desc, &name, o); } while (0)

#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb, o); } while (0)

#define INCLUDE_H(name) \
  do { glgrib_options_parser::opt o; o.hidden = true; \
       name.traverse (p + ( p == "" ? "" : ".") + #name, cb, &o); } while (0)

#define DEFINE virtual void traverse (const std::string & p, glgrib_options_callback * cb, \
                                      const glgrib_options_callback::opt * o = NULL)

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
    DESC (on,             Field is a vector);          
    DESC (hide_arrow.on,  Hide arrows);                
    DESC (hide_norm.on,   Hide norm field);            
    DESC (color,          Color for arrows);
    DESC (density,        Vector density);
    DESC (scale,          Vector scale);
    DESC (head_size,      Vector head size);
  }
  bool  on         = false;
  struct
  {
    bool on = false;
  } hide_arrow;
  struct
  {
    bool on = false;
  } hide_norm;
  glgrib_option_color color;
  float density = 50.0f;
  float scale = 1.0f;
  float head_size = 0.1f;
};

class glgrib_options_palette : public glgrib_options_base
{
public:
  static float defaultMin;
  static float defaultMax;
  DEFINE
  {
    DESC (name,        Palette name);                              
    DESC (min,         Palette min value);                              
    DESC (max,         Palette max value);                              
  }
  string name = "default";
  float min = defaultMin;
  float max = defaultMax;
};

class glgrib_options_field : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (path,                List of GRIB files);                    
    DESC (scale,               Scales to be applied to fields);        
    DESC (no_value_pointer.on, Do not keep field values in memory);    
    DESC (diff.on,             Show field difference);
    INCLUDE (palette);
    INCLUDE (vector);
    INCLUDE (contour);
  }
  string_list  path;
  float scale   = 1.0f;
  struct
  {
    bool on = false;
  } no_value_pointer;
  struct
  {
    bool on = false;
  } diff;
  glgrib_options_palette palette;
  glgrib_options_vector vector;
  glgrib_options_contour contour;
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
    DESC (flat.on,             Make Earth flat);
    DESC (orography,           Factor to apply to orography);
    DESC (path,                Path to landscape image in BMP format);
    DESC (geometry,            GRIB files to take geometry from);
    DESC (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
    DESC (wireframe.on,        Draw landscape in wireframe mode);
    DESC (position.lon1,       First longitude);
    DESC (position.lon2,       Last longitude);
    DESC (position.lat1,       First latitude);
    DESC (position.lat2,       Last latitude);
  }
  string  path  = "landscape/Whole_world_-_land_and_oceans_8000.bmp";
  float  orography  = 0.05;
  string  geometry  = "";
  int  number_of_latitudes  = 500;
  struct
  {
    bool on = false;
  } wireframe;
  bool on = false;
  struct
  {
    bool on = true;
  } flat;
  struct
  {
    float lon1 = -180.0f, lon2 = +180.0f, lat1 = -90.0f, lat2 = +90.0f;
  } position;
};

class glgrib_options_lines : public glgrib_options_base
{
public:
  glgrib_options_lines () {}
  glgrib_options_lines (const std::string & p) : path (p) {}
  DEFINE
  {
    DESC (path,               Path to lines file);
    DESC (color,              Coastlines color);
    DESC (scale,              Coastlines scale);
  }
  string path;
  glgrib_option_color color;
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
    DESC (statistics.on,      Issue statistics when window is closed);
    DESC (title,              Window title);
    DESC (debug.on,           Enable OpenGL debugging);
    DESC (version_major,      GLFW_CONTEXT_VERSION_MAJOR);
    DESC (version_minor,      GLFW_CONTEXT_VERSION_MINOR);
    INCLUDE (offscreen);
  }
  int     width  = 800;
  int     height  = 800;
  struct
  {
    bool on = false;
  } statistics;
  string  title  = "";
  struct
  {
    bool on = false;
  } debug;
  int     version_major = 4;
  int     version_minor = 3;
  glgrib_options_offscreen offscreen;
};

class glgrib_options_light : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (date_from_grib.on, Calculate light position from GRIB date);
    DESC (on,                Enable light);
    DESC (lon,               Light longitude);
    DESC (lat,               Light latitude);
    DESC (rotate.on,         Make sunlight move);
    DESC (date,              Date for sunlight position);
    DESC (frac,              Fraction of light during for night);
  }
  glgrib_option_date date;
  bool   on  = false;
  struct
  {
    bool on = false;
  } date_from_grib;
  struct
  {
    bool on = false;
  } rotate;
  float  lon  = 0.0f;
  float  lat  = 0.0f;
  float  frac = 0.1f;
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

class glgrib_options_text : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,             Enable text);
    DESC (s,              Strings to be displayed);
    DESC (x,              Coordinates of strings);
    DESC (y,              Coordinates of strings);
    DESC (a,              Text alignment);
    INCLUDE (font);
  }
  bool on = false;
  std::vector<std::string> s;
  std::vector<float> x;
  std::vector<float> y;
  std::vector<std::string> a;
  glgrib_options_font font;
};

class glgrib_options_date : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Display date);
    INCLUDE (font);
  }
  bool on = false;
  glgrib_options_font font;
};

class glgrib_options_scene : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (lon_at_hour,         Set longitude at solar time);
    DESC (rotate_earth.on,     Make earth rotate);
    INCLUDE (light);
    INCLUDE (travelling);
    DESC (test_strxyz.on,      Test XYZ string);
    INCLUDE (interpolation);
    INCLUDE (text);
    INCLUDE (image);
    INCLUDE (date);
  }
  struct
  {
    bool on  = false;
  } rotate_earth;
  float   lon_at_hour = -1.0f;
  glgrib_options_light light;  
  glgrib_options_travelling travelling;
  struct
  {
    bool on = false;
  } test_strxyz;
  glgrib_options_interpolation interpolation;
  glgrib_options_date date;
  glgrib_options_text text;
  glgrib_options_image image;
};

class glgrib_options_view : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (projection,         Mercator XYZ latlon polar_north polar_south);
    DESC (transformation,     Perspective or orthographic);
    DESC (lon,                Camera longitude);
    DESC (lat,                Camera latitude);
    DESC (fov,                Camera field of view);
    DESC (distance,           Camera distance);
    DESC (center.on,          Center view);
  }
  string  projection  = "XYZ";
  string  transformation  = "PERSPECTIVE";
  float  distance  = 6.0; 
  float  lat       = 0.0; 
  float  lon       = 0.0; 
  float  fov       = 20.;
  struct
  {
    bool on = true;
  } center;
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

class glgrib_options_rivers : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Display rivers);
    INCLUDE (lines);
  }
  bool on = false;
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/wdb_rivers_f.b");
};

class glgrib_options_border : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Display borders);
    DESC (countries.on, Display countries);
    DESC (regions.on, Display regions);
    DESC (sea.on, Display sea borders);
    INCLUDE (lines);
  }
  bool on = false;
  struct
  {
    bool on = true;
  } countries;
  struct
  {
    bool on = false;
  } regions;
  struct
  {
    bool on = false;
  } sea;
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/wdb_borders_f.b");
};

class glgrib_options_coast : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Display coastlines);
    INCLUDE (lines);
    DESC (lakes.on, Display lakes coastlines);
  }
  bool on = false;
  struct
  {
    bool on = false;
  } lakes;
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/gshhs_h.b");
};

class glgrib_options : public glgrib_options_base
{
public:
  DEFINE
  {
    INCLUDE   (field[0]); INCLUDE_H (field[1]); 
    INCLUDE_H (field[2]); INCLUDE_H (field[3]); 
    INCLUDE_H (field[4]); INCLUDE_H (field[5]); 
    INCLUDE_H (field[6]); INCLUDE_H (field[7]); 
    INCLUDE_H (field[8]); INCLUDE_H (field[9]); 
    INCLUDE (coast);
    INCLUDE (border);
    INCLUDE (rivers);
    INCLUDE (window);
    INCLUDE (landscape);
    INCLUDE (grid);
    INCLUDE (scene);
    INCLUDE (view);
    INCLUDE (colorbar);
    DESC (shell.on, Run command line);
  }
  std::vector<glgrib_options_field> field =
    {glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field ()};
  glgrib_options_coast coast;
  glgrib_options_border border;
  glgrib_options_rivers rivers;
  glgrib_options_colorbar colorbar;
  glgrib_options_window window;
  glgrib_options_landscape landscape;
  glgrib_options_grid grid;
  glgrib_options_scene scene;
  glgrib_options_view view;
  glgrib_options_font font;
  struct
  {
    bool on = false;
  } shell;
  virtual bool parse (int, const char * []);
};



#endif

