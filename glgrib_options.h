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

namespace glgrib_options_util
{
  std::string next_token (std::string *);
  std::string escape (const std::string &);
};

class glgrib_option_color
{
public:
  static glgrib_option_color color_by_name (const std::string &);
  static glgrib_option_color color_by_hexa (const std::string &);
  static void parse (glgrib_option_color *, const std::string &);

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
  friend bool operator!= (glgrib_option_color const & col1, glgrib_option_color const & col2)
  {
    return ! (col1 == col2);
  } 
};

class glgrib_option_date
{
public:
  static void parse (glgrib_option_date *, const std::string &);
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
    virtual void set (const std::string &) 
    { throw std::runtime_error (std::string ("Set method is not defined")); }
    virtual void set () 
    { throw std::runtime_error (std::string ("Set method is not defined")); }
    std::string name;
    std::string desc;
    virtual std::string type ()  = 0;
    virtual std::string asString () const = 0;
    virtual std::string asOption () const = 0;
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
    std::string asOption () const { return name + " " + glgrib_options_util::escape (asString ()); }
    void set ()
    {
    }
    void set (const std::string & v)  
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
    std::string asOption () const 
    { 
      std::string str = name;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); it++)
        {
          std::ostringstream ss;
          ss << (*it) << " ";
	  str = str + " " + glgrib_options_util::escape (std::string (ss.str ()));
	}
      return str;
    }
    std::string asString () const
    {
      std::ostringstream ss;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); it++)
        ss << (*it) << " ";
      return std::string (ss.str ());
    }
    void set (const std::string & v)  
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
  template <> std::string option_tmpl     <std::string>        ::asOption () const;
  template <> std::string option_tmpl_list<glgrib_option_color>::type ();
  template <> std::string option_tmpl_list<std::string>        ::type ();
  template <> std::string option_tmpl_list<std::string>        ::asString () const;
  template <> std::string option_tmpl_list<std::string>        ::asOption () const;
  template <> std::string option_tmpl     <bool>               ::type ();
  template <> void option_tmpl     <bool>::set        ();
  template <> void option_tmpl_list<std::string>::set (const std::string &);
  template <> void option_tmpl     <std::string>::set (const std::string &);
  template <> void option_tmpl<bool>::clear ();
  template <> std::string option_tmpl<bool>::asString () const;
  template <> std::string option_tmpl<bool>::asOption () const;
  template <> int option_tmpl<bool>::has_arg () const;

};

class glgrib_options_base;

class glgrib_options_callback
{
public:

  class opt
  {
  public:
  virtual void _dummy_ () {}
  };

#define DEF_APPLY(T) \
  virtual void apply (const std::string & path, const std::string & name, glgrib_options_base *, \
                      const std::string & desc, T * data, const opt * = NULL) {}
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
  virtual std::string asOption (glgrib_options_parser &);
  static void print (class glgrib_options &);
  bool parse (int, const char * [], const std::set<std::string> * = NULL);
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
              glgrib_options_base *, const std::string & desc, T * data,               \
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
  virtual bool parse (int, const char * [], const std::set<std::string> * = NULL);
  virtual bool parse (const char *, const std::set<std::string> * = NULL);
  virtual std::string asOption (glgrib_options_base &);
};


#define DESC(name, desc) do { cb->apply (p, #name, this, #desc, &name, o); } while (0)
#define DESC_H(name, desc) \
  do { glgrib_options_parser::opt o; o.hidden = true; \
       cb->apply (p, #name, this, #desc, &name, &o); } while (0)

#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb, o); } while (0)

#define INCLUDE_H(name) \
  do { glgrib_options_parser::opt o; o.hidden = true; \
       name.traverse (p + ( p == "" ? "" : ".") + #name, cb, &o); } while (0)

#define DEFINE virtual void traverse (const std::string & p, glgrib_options_callback * cb, \
                                      const glgrib_options_callback::opt * o = NULL)

class glgrib_options_geometry : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (triangle_strip.on,  Enable use of triangle strip);
    DESC (check.on,           Check geometry);
    DESC (height.on,          Enable height);
    DESC (height.path,        Height field path);
    DESC (height.scale,       Scale to apply to height field);
  }
  struct
  {
    bool on = true;
  } triangle_strip;
  struct
  {
    bool on = false;
  } check;
  struct
  {
    bool on = false;
    std::string path = "";
    float scale = 0.05;
  } height;
};

class glgrib_options_contour : public glgrib_options_base
{
public:
  static float defaultMin;
  static float defaultMax;
  DEFINE
  {
    DESC (number,        Number of levels);
    DESC (levels,        List of levels);
    DESC (min,           Min level);
    DESC (max,           Max level);
    DESC (widths,        List of widths);
    DESC (patterns,      List of dash patterns);
    DESC (lengths,       List of dash lengths);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin;
  float max = defaultMax;
  std::vector<float> widths;
  std::vector<std::string> patterns;
  std::vector<float> lengths;
};

class glgrib_options_stream : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (width,         Stream lines width);          
    DESC (density,       Stream lines density);          
  }
  float width = 0.0f;
  float density = 1.0f;
};

class glgrib_options_vector : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (hide_arrow.on,  Hide arrows);                
    DESC (hide_norm.on,   Hide norm field);            
    DESC (color,          Color for arrows);
    DESC (density,        Vector density);
    DESC (scale,          Vector scale);
    DESC (head_size,      Vector head size);
  }
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
  glgrib_options_palette () {}
  glgrib_options_palette (const std::string & n) : name (n) {}
  DEFINE
  {
    DESC (name,        Palette name);                              
    DESC (min,         Palette min value);                              
    DESC (max,         Palette max value);                              
    DESC (values,      Palette values);
    DESC (colors,      Palette colors);
    DESC (linear.on,   Linear colorbar display);
    DESC (scale,       Scale for colorbar figures);
  }
  string name = "default";
  float min = defaultMin;
  float max = defaultMax;
  std::vector<float> values;
  std::vector<glgrib_option_color> colors;
  struct
  {
    bool on = false;
  } linear;
  float scale = 1.0f;
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

class glgrib_options_field : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (type,                Field type : SCALAR VECTOR STREAM CONTOUR);
    DESC (user_pref.on,        Lookup field settings in database);
    DESC (path,                List of GRIB files);                    
    DESC (scale,               Scales to be applied to fields);        
    DESC (no_value_pointer.on, Do not keep field values in memory);    
    DESC (diff.on,             Show field difference);
    DESC (scalar.smooth.on,    Smooth scalar fields);
    DESC (scalar.wireframe.on, Display field as wireframe);
    DESC (scalar.points.on,    Display field as points);
    DESC (scalar.points.size.value,  Field point size);
    DESC (scalar.points.size.variable.on,  Variable field point size);
    DESC (scalar.points.size.factor.on,  Apply scale factor to point size);
    DESC (scalar.pack.bits,    Number of bytes used to pack field);
    DESC (scalar.discrete.on,  Plot as a discrete field);
    DESC (scalar.discrete.missing_color, Color for missing values);
    DESC (hilo.on,             Display low & high);
    DESC (hilo.radius,         High/low radius in degrees);
    INCLUDE (hilo.font);
    INCLUDE (palette);
    INCLUDE (vector);
    INCLUDE (contour);
    INCLUDE (stream);
    INCLUDE (geometry);
    
  }
  std::set<std::string> seen;

  struct 
  {
    bool on = false;
    glgrib_options_font font;
    float radius = 10.0f;
  } hilo;

  std::string type = "SCALAR";
  struct
  {
    bool on = true;
  } user_pref;
  struct
  {
    struct 
    {
      bool on = false;
    } smooth;
    struct 
    {
      bool on = false;
    } wireframe;
    struct 
    {
      bool on = false;
      struct
      {
        float value = 1.0f;
        struct
        {
          bool on = false;
        } variable;
        struct
        {
          bool on = true;
        } factor;
      } size;
    } points;
    struct
    {
      int bits = 8;
    } pack;
    struct
    {
      bool on = false;
      glgrib_option_color missing_color = glgrib_option_color (0, 0, 0, 0);
    } discrete;
  } scalar;
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
  glgrib_options_stream stream;
  bool parse_unseen (const char *);
  glgrib_options_geometry geometry;
};


class glgrib_options_grid : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,                Display grid);
    DESC (resolution,        Grid resolution);
    DESC (interval,          Interval between non dashed lines);
    DESC (dash_length,       Dash length in degrees);
    DESC (points,            Number of points along a parallel);
    DESC (color,             Grid color);
    DESC (scale,             Grid scale);
    DESC (labels.on,         Enable labels);
    DESC (labels.lon,        Longitude of latitude labels);
    DESC (labels.lat,        Latitude of longitude labels);
    DESC (labels.angle,      Angle of labels);
    INCLUDE (labels.font);
  }
  int resolution = 9;
  int points = 200;
  int interval = 2;
  float dash_length = 4.0f;
  glgrib_option_color color = glgrib_option_color (0, 255, 0);
  bool on = false;
  float scale = 1.005;
  struct 
  {
    bool on = false;
    float lon = 0.0f, lat = 0.0f;
    float angle = 0.0f;
    glgrib_options_font font;
  } labels;
};

class glgrib_options_landscape_position : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (lon1,       First longitude of display);
    DESC (lon2,       Last longitude of display);
    DESC (lat1,       First latitude of display);
    DESC (lat2,       Last latitude of display);
  }
  float lon1 = -180.0f, lon2 = +180.0f, lat1 = -90.0f, lat2 = +90.0f;
};

class glgrib_options_landscape : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on,                  Enable landscape);
    DESC (projection,          Projection : LONLAT or WEBMERCATOR);
    DESC (flat.on,             Make Earth flat);
    DESC (path,                Path to landscape image in BMP format);
    DESC (geometry_path,       GRIB files to take geometry from);
    DESC (number_of_latitudes, Number of latitudes used for creating a mesh for the landscape);
    DESC (wireframe.on,        Draw landscape in wireframe mode);
    INCLUDE (lonlat.position);
    INCLUDE (geometry);
  }
  string projection = "LONLAT";
  string path  = "landscape/Whole_world_-_land_and_oceans_08000.bmp";
  float  orography  = 0.05;
  string geometry_path = "";
  int number_of_latitudes  = 500;
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
    glgrib_options_landscape_position position;
  } lonlat;
  glgrib_options_geometry geometry;
};

class glgrib_options_lines : public glgrib_options_base
{
public:
  glgrib_options_lines () {}
  glgrib_options_lines (const std::string & p, const string & f) : path (p), format (f) {}
  DEFINE
  {
    DESC (path,               Path to lines file);
    DESC (color,              Coastlines color);
    DESC (scale,              Coastlines scale);
    DESC (format,             Input format);
    DESC (selector,           Selector);
    DESC (latmin,             Minimum latitude);
    DESC (latmax,             Maximum latitude);
    DESC (lonmin,             Minimum longitude);
    DESC (lonmax,             Maximum longitude);
  }
  std::string selector;
  string path;
  string format = "gshhg";
  glgrib_option_color color;
  float scale = 1.005;
  float latmin = 0.0f, latmax = 0.0f, lonmin = 0.0f, lonmax = 0.0f;
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
    DESC (width,                 Window width);
    DESC (height,                Window height);
    DESC (statistics.on,         Issue statistics when window is closed);
    DESC (antialiasing.on,       Enable antialiasing);
    DESC (antialiasing.samples,  Samples for antialiasing);
    DESC (title,                 Window title);
    DESC (debug.on,              Enable OpenGL debugging);
    DESC (version_major,         GLFW_CONTEXT_VERSION_MAJOR);
    DESC (version_minor,         GLFW_CONTEXT_VERSION_MINOR);
    INCLUDE (offscreen);
    DESC (info.on,               Show hardware info);
    DESC_H (fix_landscape.on,    Fix landscape position);
  }
  int     width   = 800;
  int     height  = 800;
  struct
  {
    bool on = true;
    int samples = 4;
  } antialiasing;
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
  struct
  {
    bool on = false;
  } fix_landscape;
  struct
  {
    bool on = false;
  } info;
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
    DESC (night,             Fraction of light during for night);
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
  float  night = 0.1f;
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

class glgrib_options_title : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, "Enable title");
    DESC (x,  "Coordinates");
    DESC (y,  "Coordinates");
    DESC (a,  "Alignment");
    INCLUDE (font);
    DESC (text, "Title");
  }
  bool on = false;
  float x = 0.;
  float y = 1.;
  std::string a = "NW";
  std::string text = "";
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
    INCLUDE (title);
    DESC (select.field, Rank of field to select);
    DESC (center.on, Center on first field);
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
  glgrib_options_title title;
  struct
  {
    int field = 0;
  } select;
  struct
  {
    bool on = false;
  } center;
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
    DESC (clip.on,            Enable Mercator and lat/lon clippling);
    DESC (clip.dlon,          Amount of longitude to clip);
    DESC (clip.dlat,          Amount of latitude to clip);
    DESC (clip.xmin,          Min viewport x coordinate);
    DESC (clip.xmax,          Max viewport x coordinate);
    DESC (clip.ymin,          Min viewport y coordinate);
    DESC (clip.ymax,          Max viewport y coordinate);
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
  struct
  {
    float dlon = 10.0f;
    float dlat =  5.0f;
    float xmin = 0.0f, xmax = 1.0f, ymin = 0.0f, ymax = 1.0f;
    bool on = true;
  } clip;
};


class glgrib_options_colorbar : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Activate colorbar);
    INCLUDE (font);
    DESC (levels.number, Colorbar number of levels);
    DESC (levels.values, Colorbar level values);
    DESC (format, "Format (sprintf) use to display numbers");
    DESC (position.xmin, Colorbar position);
    DESC (position.xmax, Colorbar position); 
    DESC (position.ymin, Colorbar position); 
    DESC (position.ymax, Colorbar position);
  }
  bool on = false;
  glgrib_options_font font = glgrib_options_font ("fonts/16.bmp", 0.02f);
  std::string format = "%6.4g";
  struct
  {
    int number = 11;
    std::vector<float> values;
  } levels;
  struct position
  {
    float xmin = 0.08;
    float xmax = 0.18; 
    float ymin = 0.05; 
    float ymax = 0.95;
  } position;
};

class glgrib_options_mapscale : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Activate mapscale);
    INCLUDE (font);
    DESC (position.xmin, Mapscale position);
    DESC (position.xmax, Mapscale position); 
    DESC (position.ymin, Mapscale position); 
    DESC (position.ymax, Mapscale position);
    DESC (color1, First color);
    DESC (color2, Second color);
  }
  bool on = false;
  glgrib_options_font font = glgrib_options_font ("fonts/16.bmp", 0.02f);
  glgrib_option_color color1 = glgrib_option_color (255, 255, 255);
  glgrib_option_color color2 = glgrib_option_color ( 80,  80,  80);
  struct position
  {
    float xmin = 0.05;
    float xmax = 0.25; 
    float ymin = 0.05; 
    float ymax = 0.07;
  } position;
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
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/gshhg/WDBII_bin/wdb_rivers_f.b", "gshhg");
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
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/gshhg/WDBII_bin/wdb_borders_f.b", "gshhg");
};

class glgrib_options_points : public glgrib_options_base
{
public:
  DEFINE 
  {
    DESC (scale, Scale);
    DESC (size.variable.on, Enable variable point size);
    DESC (size.value, Point size);
    INCLUDE (palette);
    DESC (color, Point color);
  }
  glgrib_options_palette palette = glgrib_options_palette ("none");
  glgrib_option_color color;
  float scale = 1.0f;
  struct
  {
    float value = 1.0f;
    struct
    {
       bool on = false;
    } variable;
  } size;
};

class glgrib_options_cities : public glgrib_options_base
{
public:
  DEFINE 
  {
    DESC (on, Display cities);
    INCLUDE (points);
    INCLUDE (labels.font);
    DESC (labels.on, Enable city names display);
  }
  bool on = false;
  glgrib_options_points points;
  struct
  {
    glgrib_options_font font;
    bool on = false;
  } labels;
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
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/gshhg/GSHHS_bin/gshhs_h.b", "gshhg");
};

class glgrib_options_departements : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Display departements);
    INCLUDE (lines);
  }
  bool on = false;
  glgrib_options_lines lines = glgrib_options_lines ("coastlines/departements/departements-20180101.shp", "shapeline");
};

class glgrib_options_shell : public glgrib_options_base
{
public:
  DEFINE
  {
    DESC (on, Run command line);
    DESC_H (script, Script to execute);
    DESC (prompt.on, Start interactive session after batch script completion);
  }
  bool on = false; 
  std::string script;
  struct
  {
    bool on = false;
  } prompt;
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
    INCLUDE (cities);
    INCLUDE (border);
    INCLUDE (rivers);
    INCLUDE (window);
    INCLUDE (landscape);
    INCLUDE (grid);
    INCLUDE (scene);
    INCLUDE (view);
    INCLUDE (colorbar);
    INCLUDE (mapscale);
    INCLUDE (departements);
    INCLUDE (shell);
  }
  std::vector<glgrib_options_field> field =
    {glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field (), 
     glgrib_options_field (), glgrib_options_field ()};
  glgrib_options_coast coast;
  glgrib_options_cities cities;
  glgrib_options_border border;
  glgrib_options_rivers rivers;
  glgrib_options_colorbar colorbar;
  glgrib_options_mapscale mapscale;
  glgrib_options_departements departements;
  glgrib_options_window window;
  glgrib_options_landscape landscape;
  glgrib_options_grid grid;
  glgrib_options_scene scene;
  glgrib_options_view view;
  glgrib_options_font font;
  glgrib_options_shell shell;
  virtual bool parse (int, const char * [], const std::set<std::string> * = NULL);
};



#endif

