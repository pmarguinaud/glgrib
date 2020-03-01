#pragma once

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
#include <string.h>

namespace glgrib_options_util
{
  std::string next_token (std::string *);
  std::string escape (const std::string &);
};

class glGribOptionColor
{
public:
  static glGribOptionColor colorByName (const std::string &);
  static glGribOptionColor colorByHexa (const std::string &);
  static void parse (glGribOptionColor *, const std::string &);

  glGribOptionColor () {}
  glGribOptionColor (int _r, int _g, int _b, int _a = 255) : r (_r), g (_g), b (_b), a (_a) {}
  glGribOptionColor (const std::string &);

  int r = 255, g = 255, b = 255, a = 255;
  std::string asString () const 
  {
    char str[32]; 
    sprintf (str, "#%2.2x%2.2x%2.2x%2.2x", r, g, b, a); 
    return std::string (str); 
  }
  friend std::ostream & operator << (std::ostream &, const glGribOptionColor &);
  friend std::istream & operator >> (std::istream &, glGribOptionColor &);
  friend bool operator== (glGribOptionColor const & col1, glGribOptionColor const & col2)
  {
    return (col1.r == col2.r)
        && (col1.g == col2.g)
        && (col1.b == col2.b)
        && (col1.a == col2.a);
  }
  friend bool operator!= (glGribOptionColor const & col1, glGribOptionColor const & col2)
  {
    return ! (col1 == col2);
  } 
};

class glGribOptionDate
{
public:
  static void parse (glGribOptionDate *, const std::string &);
  glGribOptionDate () {}
  glGribOptionDate (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  static glGribOptionDate interpolate (const glGribOptionDate &, const glGribOptionDate &, const float);
  static glGribOptionDate date_from_t (time_t);
  static time_t t_from_date (const glGribOptionDate &);
  std::string asString () const;
  friend std::ostream & operator << (std::ostream &, const glGribOptionDate &);
  friend std::istream & operator >> (std::istream &, glGribOptionDate &);
  friend bool operator== (glGribOptionDate const & d1, glGribOptionDate const & d2)
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
    option_tmpl (const std::string & n, const std::string & d, T * v = nullptr) : option_base (n, d), value (v) {}
    T * value = nullptr;
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
      const option_tmpl<T> * o = nullptr;
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
    option_tmpl_list (const std::string & n, const std::string & d, std::vector<T> * v = nullptr) : option_base (n, d), value (v) {}
    std::vector<T> * value = nullptr;
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
      const option_tmpl_list<T> * o = nullptr;
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
  template <> std::string option_tmpl     <glGribOptionDate> ::type ();
  template <> std::string option_tmpl     <glGribOptionColor>::type ();
  template <> std::string option_tmpl     <std::string>        ::type ();
  template <> std::string option_tmpl     <std::string>        ::asString () const;
  template <> std::string option_tmpl     <std::string>        ::asOption () const;
  template <> std::string option_tmpl_list<glGribOptionColor>::type ();
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

class glGribOptionsBase;

class glGribOptionsCallback
{
public:

  class opt
  {
  public:
  virtual void _dummy_ () {}
  };

#define DEF_APPLY(T) \
  virtual void apply (const std::string & path, const std::string & name, glGribOptionsBase *, \
                      const std::string & desc, T * data, const opt * = nullptr) {}
  DEF_APPLY (float);
  DEF_APPLY (bool);
  DEF_APPLY (int);
  DEF_APPLY (std::vector<std::string>);
  DEF_APPLY (std::string);
  DEF_APPLY (std::vector<float>);
  DEF_APPLY (std::vector<int>);
  DEF_APPLY (glGribOptionColor);
  DEF_APPLY (std::vector<glGribOptionColor>);
  DEF_APPLY (glGribOptionDate);
#undef DEF_APPLY
};


class glGribOptionsParser : public glGribOptionsCallback
{
public:
  virtual std::string asOption (glGribOptionsParser &);
  static void print (class glGribOptions &);
  bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  void show_help ();
  void display (const std::string &, bool = false);
  ~glGribOptionsParser ()
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
    return nullptr;
  }

  class opt : public glGribOptionsCallback::opt
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
                     const glGribOptionsCallback::opt * _o)
  {
    const opt * o = dynamic_cast<const opt*>(_o);
    if (o != nullptr)
      option->hidden = o->hidden;
    name2option.insert (opt_name, option);
  }

#define DEF_APPLY(T,C) \
  void apply (const std::string & path, const std::string & name,                      \
              glGribOptionsBase *, const std::string & desc, T * data,               \
              const glGribOptionsCallback::opt * o = nullptr)                           \
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
  DEF_APPLY (glGribOptionColor               , glgrib_options_parser_detail::option_tmpl<glGribOptionColor>);
  DEF_APPLY (std::vector<glGribOptionColor>  , glgrib_options_parser_detail::option_tmpl_list<glGribOptionColor>);
  DEF_APPLY (glGribOptionDate                , glgrib_options_parser_detail::option_tmpl<glGribOptionDate>);

#undef DEF_APPLY

};

class glGribOptionsBase 
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<float> float_list;
  typedef std::string string;
  virtual void traverse (const std::string &, glGribOptionsCallback *, 
                         const glGribOptionsCallback::opt * = nullptr) {}
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  virtual bool parse (const char *, const std::set<std::string> * = nullptr);
  virtual std::string asOption (glGribOptionsBase &);
};


#define DESC(name, desc) do { cb->apply (p, #name, this, #desc, &name, o); } while (0)
#define DESC_H(name, desc) \
  do { glGribOptionsParser::opt o; o.hidden = true; \
       cb->apply (p, #name, this, #desc, &name, &o); } while (0)

#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb, o); } while (0)

#define INCLUDE_H(name) \
  do { glGribOptionsParser::opt o; o.hidden = true; \
       name.traverse (p + ( p == "" ? "" : ".") + #name, cb, &o); } while (0)

#define DEFINE virtual void traverse (const std::string & p, glGribOptionsCallback * cb, \
                                      const glGribOptionsCallback::opt * o = nullptr)

class glGribOptionsGeometry : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (triangle_strip.on,  Enable use of triangle strip);
    DESC (check.on,           Check geometry);
    DESC (height.on,          Enable height);
    DESC (height.path,        Height field path);
    DESC (height.scale,       Scale to apply to height field);
    DESC (frame.on,           Draw frame around domain);
    DESC (frame.color1,       First frame color);
    DESC (frame.color2,       Second frame color);
    DESC (frame.width,        Frame width);
    DESC (frame.dlon,         Longitude interval);
    DESC (frame.dlat,         Latitude interval);
    DESC (gencoords.on,       Generate coordinates on GPU when possible);
    DESC (gaussian.fit.on,    Fit Gaussian latitude retrieval);
  }
  struct
  {
    bool on = false;
  } gencoords;
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
  struct 
  {
    bool on = false;
    glGribOptionColor color1 = glGribOptionColor (255, 255, 255);
    glGribOptionColor color2 = glGribOptionColor ( 80,  80,  80);
    float width = 0.0f;
    float dlon = 10.0f;
    float dlat = 10.0f;
  } frame;
  struct
  {
    struct
    {
      bool on = false;
    } fit;
  } gaussian;
};

class glGribOptionsFont : public glGribOptionsBase
{
public:
  glGribOptionsFont (const std::string & b, float s) : bitmap (b), scale (s) {}
  glGribOptionsFont (float s) : scale (s) {}
  glGribOptionsFont () {}
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
    glGribOptionColor foreground = glGribOptionColor (255, 255, 255, 255);
    glGribOptionColor background = glGribOptionColor (  0,   0,   0,   0);
  } color;
};

class glGribOptionsContour : public glGribOptionsBase
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
    DESC (labels.on,     Enable labels);
    INCLUDE (labels.font);
    DESC (labels.distmin, Minimal length in degrees for labelled lines);
    DESC (labels.format,  Format to print labels);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin;
  float max = defaultMax;
  std::vector<float> widths;
  std::vector<std::string> patterns;
  std::vector<float> lengths;
  struct
  {
    bool on = false;
    glGribOptionsFont font;
    float distmin = 3.0f;
    std::string format = "%12.2f";
  } labels;
};

class glGribOptionsIsofill : public glGribOptionsBase
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
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin;
  float max = defaultMax;
};

class glGribOptionsStream : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (width,         Stream lines width);          
    DESC (density,       Stream lines density);          
    DESC (motion.on,     Enable stream lines motion);
    DESC (motion.timeaccel, Time acceleration);
    DESC (motion.nwaves,    Number of waves over 1000km);
  }
  float width = 0.0f;
  float density = 1.0f;
  struct
  {
    bool on = false;
    float timeaccel = 10000.0f;
    float nwaves = 10.f;
  } motion;
};

class glGribOptionsVector : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (arrow.on,        Show arrows);                
    DESC (norm.on,         Show norm field);            
    DESC (arrow.color,     Color for arrows);
    DESC (density,         Vector density);
    DESC (scale,           Vector scale);
    DESC (arrow.head_size, Vector head size);
    DESC (arrow.kind,      Arrow kind);
    DESC (arrow.fill.on,   Fill arrow);
    DESC (arrow.fixed.on,  Fixed sized arrow);
    DESC (arrow.min,       Vector min value);
    DESC (barb.on,         Enable barbs);
    DESC (barb.color,      Barbs color);
    DESC (barb.angle,      Barbs angle);
    DESC (barb.length,     Barbs length);
    DESC (barb.dleng,      Barbs length);
    DESC (barb.levels,     Barbs levels);
    DESC (barb.circle.level,   Value under which we draw a circle);
    DESC (barb.pennant.levels, Pennants levels);
  }
  struct
  {
    bool on = true;
    float head_size = 0.1f;
    int kind = 0;
    struct
    {
      bool on = false;
    } fill;
    glGribOptionColor color;
    struct
    {
      bool on = false;
    } fixed;
    float min = 0.0f;
  } arrow;
  struct
  {
    bool on = false;
    glGribOptionColor color;
    float angle  = 60.0;
    float length = 0.25;
    float dleng  = 0.1;
    std::vector<float> levels = {0., 5., 10., 15., 20., 25.};
    struct
    {
      float level = 1.0f;
    } circle;
    struct
    {
      std::vector<float> levels = {0., 25., 50., 75.};
    } pennant;
  } barb;
  struct
  {
    bool on = true;
  } norm;
  float density = 50.0f;
  float scale = 1.0f;
};

class glGribOptionsPalette : public glGribOptionsBase
{
public:
  static float defaultMin;
  static float defaultMax;
  glGribOptionsPalette () {}
  glGribOptionsPalette (const std::string & n) : name (n) {}
  DEFINE
  {
    DESC (name,        Palette name);                              
    DESC (min,         Palette min value);                              
    DESC (max,         Palette max value);                              
    DESC (values,      Palette values);
    DESC (colors,      Palette colors);
    DESC (linear.on,   Linear colorbar display);
    DESC (scale,       Scale for colorbar figures);
    DESC (offset,      Offset for colorbar figures);
    DESC (rainbow.on,  Generate rainbow palette);
    DESC (rainbow.direct.on,  Direct/indirect rainbow);
    DESC (generate.on,        Generate values);
    DESC (generate.levels,    Number of values to generate);
  }
  string name = "default";
  float min = defaultMin;
  float max = defaultMax;
  std::vector<float> values;
  std::vector<glGribOptionColor> colors;
  struct
  {
    bool on = false;
  } linear;
  struct
  {
    bool on = false;
    struct
    {
      bool on = false;
    } direct;
  } rainbow;
  struct
  {
    bool on = false;
    int levels = 10;
  } generate;
  float scale = 1.0f;
  float offset = 0.0f;
};

class glGribOptionsScalar : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (smooth.on,    Smooth scalar fields);
    DESC (wireframe.on, Display field as wireframe);
    DESC (points.on,    Display field as points);
    DESC (points.size.value,  Field point size);
    DESC (points.size.variable.on,  Variable field point size);
    DESC (points.size.factor.on,  Apply scale factor to point size);
    DESC (pack.bits,    Number of bytes used to pack field);
    DESC (discrete.on,  Plot as a discrete field);
    DESC (discrete.missing_color, Color for missing values);
  }

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
    glGribOptionColor missing_color = glGribOptionColor (0, 0, 0, 0);
  } discrete;
};

class glGribOptionsMpiview : public glGribOptionsBase
{
public:
  DEFINE
  { 
    DESC (path,  "Path to MPI distribution field");
    DESC (on,    "Enable MPI view");
    DESC (scale, "Displacement scale");
  }

  std::vector<std::string> path;
  bool on = false;
  float scale = 0.1f;
};

class glGribOptionsField : public glGribOptionsBase
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
    DESC (hilo.on,             Display low & high);
    DESC (hilo.radius,         High/low radius in degrees);
    INCLUDE (hilo.font);
    INCLUDE (palette);
    INCLUDE (scalar);
    INCLUDE (vector);
    INCLUDE (contour);
    INCLUDE (isofill);
    INCLUDE (stream);
    INCLUDE (geometry);
    INCLUDE_H (mpiview);
  }
  std::set<std::string> seen;

  struct 
  {
    bool on = false;
    glGribOptionsFont font;
    float radius = 10.0f;
  } hilo;

  std::string type = "SCALAR";
  struct
  {
    bool on = true;
  } user_pref;
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
  glGribOptionsPalette palette;
  glGribOptionsScalar scalar;
  glGribOptionsVector vector;
  glGribOptionsContour contour;
  glGribOptionsIsofill isofill;
  glGribOptionsStream stream;
  bool parse_unseen (const char *);
  glGribOptionsGeometry geometry;
  glGribOptionsMpiview mpiview;
};


class glGribOptionsTicksSide : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable side);
    DESC (dlon, Longitude interval);
    DESC (dlat, Latitude interval);
    DESC (nswe.on, North/South/West/East labels instead of +/-);
  }
  bool on = true;
  float dlon = 10.0f;
  float dlat = 10.0f;
  struct
  {
    bool on = false;
  } nswe;
};

class glGribOptionsTicks : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (lines.on,                Display ticks);
    DESC (lines.color,             Tick color);
    DESC (lines.length,            Tick length);
    DESC (lines.width,             Tick width);
    DESC (lines.kind,              Tick kind);
    DESC (labels.on,               Display tick labels);
    DESC (labels.format,           Format for tick labels);
    INCLUDE (labels.font);
    DESC (frame.on,                Enable frame);
    DESC (frame.width,             Frame width);
    DESC (frame.color,             Frame color);
    INCLUDE (N);
    INCLUDE (S);
    INCLUDE (W);
    INCLUDE (E);
  }
  struct 
  {
    bool on = false;
    glGribOptionsFont font = glGribOptionsFont (0.02f);
    std::string format = "%+06.2f";
  } labels;
  struct
  {
    bool on = false;
    float width = 0.01f;
    glGribOptionColor color = glGribOptionColor (255,   0,   0);
  } frame;
  struct
  {
    bool on = false;
    glGribOptionColor color = glGribOptionColor (255, 255, 255);
    float length = 0.025f;
    float width  = 0.010f;
    int kind     = 0;
  } lines;
  glGribOptionsTicksSide N, S, W, E;
};

class glGribOptionsGrid : public glGribOptionsBase
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
  glGribOptionColor color = glGribOptionColor (0, 255, 0);
  bool on = false;
  float scale = 1.005;
  struct 
  {
    bool on = false;
    float lon = 0.0f, lat = 0.0f;
    float angle = 0.0f;
    glGribOptionsFont font;
  } labels;
};

class glGribOptionsLandLayer : public glGribOptionsBase
{
public:
  glGribOptionsLandLayer (const std::string & _path, float _scale, const glGribOptionColor & _color)
                           :  path (_path), scale (_scale), color (_color) {}
  DEFINE
  {
    DESC (selector,           "Shape selection");
    DESC (path,               "Path to coastlines");
    DESC (subdivision.angle,  "Angle max for subdivision");
    DESC (subdivision.on,     "Enable subdivision");
    DESC (on,                 "Enable");
    DESC (scale,              "Scale");
    DESC (color,              "Land color");
    DESC_H (debug.on,         "Debug");
  }
  std::string selector = "";
  std::string path     = "coastlines/shp/GSHHS_c_L1.shp";
  float scale          = 1.0f;
  struct
  {
    float angle        = 1.0f;
    bool on            = true;
  } subdivision;
  glGribOptionColor color = glGribOptionColor ("#ffe2ab");
  struct
  {
    bool on            = false;
  } debug;
  bool on              = true;
};

class glGribOptionsLand : public glGribOptionsBase
{
public:
  DEFINE
  {
    INCLUDE   (layers[0]);
    INCLUDE_H (layers[1]);
    INCLUDE_H (layers[2]);
    INCLUDE_H (layers[3]);
    DESC (on, "Enable land");
  }
  
  bool on = false;

  std::vector<glGribOptionsLandLayer> layers = 
  {
    glGribOptionsLandLayer ("coastlines/shp/GSHHS_c_L1.shp", 1.000f, glGribOptionColor ("#ffe2ab")),
    glGribOptionsLandLayer ("coastlines/shp/GSHHS_c_L2.shp", 1.001f, glGribOptionColor ("#0000ff")),
    glGribOptionsLandLayer ("coastlines/shp/GSHHS_c_L3.shp", 1.002f, glGribOptionColor ("#ffe2ab")),
    glGribOptionsLandLayer ("coastlines/shp/GSHHS_c_L5.shp", 1.000f, glGribOptionColor ("#ffe2ab")) 
  };

};

class glGribOptionsLandscapePosition : public glGribOptionsBase
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

class glGribOptionsLandscape : public glGribOptionsBase
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
    DESC (scale,               Scale);
    DESC (color,               Color);
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
    glGribOptionsLandscapePosition position;
  } lonlat;
  glGribOptionsGeometry geometry;
  float scale = 1.0f;
  glGribOptionColor color = glGribOptionColor ("#00000000");
};

class glGribOptionsLines : public glGribOptionsBase
{
public:
  glGribOptionsLines () {}
  glGribOptionsLines (const std::string & p, const string & f) : path (p), format (f) {}
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
  glGribOptionColor color;
  float scale = 1.005;
  float latmin = 0.0f, latmax = 0.0f, lonmin = 0.0f, lonmax = 0.0f;
};

class glGribOptionsOffscreen : public glGribOptionsBase
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

class glGribOptionsWindow : public glGribOptionsBase
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
    DESC (position.x,            Window x position);
    DESC (position.y,            Window y position);
  }
  int     width   = 800;
  int     height  = 800;
  struct
  {
    int x = -1, y = -1;
  } position;
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
  glGribOptionsOffscreen offscreen;
  struct
  {
    bool on = false;
  } fix_landscape;
  struct
  {
    bool on = false;
  } info;
};

class glGribOptionsLight : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (date_from_grib.on, Calculate light position from GRIB date);
    DESC (on,                Enable light);
    DESC (lon,               Light longitude);
    DESC (lat,               Light latitude);
    DESC (rotate.on,         Make sunlight move);
    DESC (rotate.rate,       Rate of rotation : angle/frame);
    DESC (date,              Date for sunlight position);
    DESC (night,             Fraction of light during for night);
  }
  glGribOptionDate date;
  bool   on  = false;
  struct
  {
    bool on = false;
  } date_from_grib;
  struct
  {
    bool on = false;
    float rate = 1.0f;
  } rotate;
  float  lon  = 0.0f;
  float  lat  = 0.0f;
  float  night = 0.1f;
};

class glGribOptionsPosition : public glGribOptionsBase
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

class glGribOptionsTravelling : public glGribOptionsBase
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
  glGribOptionsPosition pos1;
  glGribOptionsPosition pos2;
};

class glGribOptionsInterpolation : public glGribOptionsBase
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

class glGribOptionsImage : public glGribOptionsBase
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

class glGribOptionsText : public glGribOptionsBase
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
  glGribOptionsFont font;
};

class glGribOptionsDate : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display date);
    INCLUDE (font);
  }
  bool on = false;
  glGribOptionsFont font;
};

class glGribOptionsTitle : public glGribOptionsBase
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
  glGribOptionsFont font;
};

class glGribOptionsScene : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (lon_at_hour,         Set longitude at solar time);
    DESC (rotate_earth.on,     Make earth rotate);
    DESC (rotate_earth.rate,   Rate of rotation : angle/frame);
    INCLUDE (light);
    INCLUDE (travelling);
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
    float rate = 1.0f;
  } rotate_earth;
  float   lon_at_hour = -1.0f;
  glGribOptionsLight light;  
  glGribOptionsTravelling travelling;
  glGribOptionsInterpolation interpolation;
  glGribOptionsDate date;
  glGribOptionsText text;
  glGribOptionsImage image;
  glGribOptionsTitle title;
  struct
  {
    int field = 0;
  } select;
  struct
  {
    bool on = false;
  } center;
};

class glGribOptionsView : public glGribOptionsBase
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

  friend bool operator== (const glGribOptionsView & v1, const glGribOptionsView & v2)
  {
#define EQ(a) if (v1.a != v2.a) return false;
    EQ (projection);
    EQ (transformation);
    EQ (lon);
    EQ (lat);
    EQ (fov);
    EQ (distance);
    EQ (center.on);
    EQ (clip.on);
    EQ (clip.dlon);
    EQ (clip.dlat);
    EQ (clip.xmin);
    EQ (clip.xmax);
    EQ (clip.ymin);
    EQ (clip.ymax);
#undef EQ
    return true;
  }
};


class glGribOptionsColorbar : public glGribOptionsBase
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
  glGribOptionsFont font = glGribOptionsFont ("fonts/16.bmp", 0.02f);
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

class glGribOptionsMapscale : public glGribOptionsBase
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
  glGribOptionsFont font = glGribOptionsFont ("fonts/16.bmp", 0.02f);
  glGribOptionColor color1 = glGribOptionColor (255, 255, 255);
  glGribOptionColor color2 = glGribOptionColor ( 80,  80,  80);
  struct position
  {
    float xmin = 0.05;
    float xmax = 0.25; 
    float ymin = 0.05; 
    float ymax = 0.07;
  } position;
};

class glGribOptionsRivers : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display rivers);
    INCLUDE (lines);
  }
  bool on = false;
  glGribOptionsLines lines = glGribOptionsLines ("coastlines/gshhg/WDBII_bin/wdb_rivers_f.b", "gshhg");
};

class glGribOptionsBorder : public glGribOptionsBase
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
  glGribOptionsLines lines = glGribOptionsLines ("coastlines/gshhg/WDBII_bin/wdb_borders_f.b", "gshhg");
};

class glGribOptionsPoints : public glGribOptionsBase
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
  glGribOptionsPalette palette = glGribOptionsPalette ("none");
  glGribOptionColor color;
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

class glGribOptionsCities : public glGribOptionsBase
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
  glGribOptionsPoints points;
  struct
  {
    glGribOptionsFont font;
    bool on = false;
  } labels;
};

class glGribOptionsCoast : public glGribOptionsBase
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
  glGribOptionsLines lines = glGribOptionsLines ("coastlines/gshhg/GSHHS_bin/gshhs_h.b", "gshhg");
};

class glGribOptionsDepartements : public glGribOptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display departements);
    INCLUDE (lines);
  }
  bool on = false;
  glGribOptionsLines lines = glGribOptionsLines ("coastlines/departements/departements-20180101.shp", "shapeline");
};

class glGribOptionsShell : public glGribOptionsBase
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

class glGribOptions : public glGribOptionsBase
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
    INCLUDE (ticks);
    INCLUDE (scene);
    INCLUDE (view);
    INCLUDE (colorbar);
    INCLUDE (mapscale);
    INCLUDE (departements);
    INCLUDE (shell);
    INCLUDE (land);
    DESC (diff.on, "Enable difference mode");
    DESC (diff.path, "Files to show in diff mode");
  }
  struct
  {
    std::vector<std::string> path;
    bool on = false;
  } diff;
  std::vector<glGribOptionsField> field =
    {glGribOptionsField (), glGribOptionsField (), 
     glGribOptionsField (), glGribOptionsField (), 
     glGribOptionsField (), glGribOptionsField (), 
     glGribOptionsField (), glGribOptionsField (), 
     glGribOptionsField (), glGribOptionsField ()};
  glGribOptionsCoast coast;
  glGribOptionsCities cities;
  glGribOptionsBorder border;
  glGribOptionsRivers rivers;
  glGribOptionsColorbar colorbar;
  glGribOptionsMapscale mapscale;
  glGribOptionsDepartements departements;
  glGribOptionsWindow window;
  glGribOptionsLandscape landscape;
  glGribOptionsGrid grid;
  glGribOptionsTicks ticks;
  glGribOptionsScene scene;
  glGribOptionsView view;
  glGribOptionsFont font;
  glGribOptionsShell shell;
  glGribOptionsLand land;
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
};


#undef DESC
#undef INCLUDE
#undef INCLUDE_H
