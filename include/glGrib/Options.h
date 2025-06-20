#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <time.h>


namespace glGrib
{

namespace OptionsUtil
{
  const std::string nextToken (std::string *);
  const std::string escape (const std::string &);
};


class OptionFloatLike
{
public:
  OptionFloatLike () {}
  OptionFloatLike (float s) : value (s) {}

  operator float () const
  {
    return value;
  }

  float operator + (float s)
  {
    return value + s;
  }

  float operator - (float s)
  {
    return value - s;
  }

  OptionFloatLike & operator += (float s)
  { 
    value += s;
    return *this;
  }

  OptionFloatLike & operator -= (float s)
  { 
    value -= s;
    return *this;
  }

  friend bool operator == (const OptionFloatLike & s1, const OptionFloatLike & s2)
  {  
    return s1.value == s2.value;
  }

  friend bool operator != (const OptionFloatLike & s1, const OptionFloatLike & s2)
  {  
    return ! (s1 == s2);
  }

  friend std::ostream & operator << (std::ostream & out, const OptionFloatLike & osl)
  {
    return out << osl.value;
  }

  friend std::istream & operator >> (std::istream & in, OptionFloatLike & osl)
  {
    in >> osl.value;
    return in;
  }

private:
  float value = 0.0f;
};

#define DEF_OPTION_FLOAT(c) \
class c : public OptionFloatLike \
{ public: using OptionFloatLike::OptionFloatLike; }


DEF_OPTION_FLOAT (OptionScale);
DEF_OPTION_FLOAT (OptionLongitude);
DEF_OPTION_FLOAT (OptionLatitude);
DEF_OPTION_FLOAT (OptionAngle);

class OptionStringLike
{
public:
  OptionStringLike () {}
  OptionStringLike (const std::string & p) : value (p) {}
  OptionStringLike (const char * p) : value (p) {}

  operator const std::string & () const
  {
    return value;
  }
  operator std::string ()
  {
    return value;
  }
  const std::string * operator -> () const
  {
    return &value;
  }
  std::string * operator -> ()
  {
    return &value;
  }
  const std::string asString () const
  {
    return value;
  }
  const std::string asJSON () const;

  friend std::ostream & operator << (std::ostream & out, const OptionStringLike & osl)
  {
    return out << osl.value;
  }

  friend std::istream & operator >> (std::istream & in, OptionStringLike & osl)
  {
    in >> osl.value;
    return in;
  }

  friend bool operator== (OptionStringLike const & osl1, const char * osl2)
  {
    return osl1.value == std::string (osl2);
  }
  friend bool operator== (OptionStringLike const & osl1, OptionStringLike const & osl2)
  {
    return osl1.value == osl2.value;
  }
  friend bool operator!= (OptionStringLike const & osl1, const char * osl2)
  {
    return ! (osl1 == osl2);
  } 
  friend bool operator!= (OptionStringLike const & osl1, OptionStringLike const & osl2)
  {
    return ! (osl1 == osl2);
  } 
  std::string operator+ (const std::string & s) const
  {
    return value + s;
  }
  friend std::string operator + (const std::string & s, const OptionStringLike & osl)
  {
    return s + osl.value;
  }
private:
  std::string value;
};


#define DEF_OPTION_STRING(c) \
class c : public OptionStringLike \
{ public: using OptionStringLike::OptionStringLike; }

DEF_OPTION_STRING (OptionFieldType);
DEF_OPTION_STRING (OptionPath);
DEF_OPTION_STRING (OptionProjection);
DEF_OPTION_STRING (OptionTransformation);
DEF_OPTION_STRING (OptionFieldRef);
DEF_OPTION_STRING (OptionPaletteName);

class OptionColor
{
public:
  static OptionColor colorByName (const std::string &);
  static OptionColor colorByHexa (const std::string &);
  static void parse (OptionColor *, const std::string &);

  OptionColor () {}
  explicit OptionColor (int _r, int _g, int _b, int _a = 255) : r (_r), g (_g), b (_b), a (_a) {}
  explicit OptionColor (const std::string &);

  int r = 255, g = 255, b = 255, a = 255;
  const std::string asString () const 
  {
    char str[32]; 
    sprintf (str, "#%2.2x%2.2x%2.2x%2.2x", r, g, b, a); 
    return std::string (str); 
  }
  const std::string asJSON () const 
  {
    return std::string ("\"") + asString () + std::string ("\"");
  }
  friend std::ostream & operator << (std::ostream &, const OptionColor &);
  friend std::istream & operator >> (std::istream &, OptionColor &);
  friend bool operator== (OptionColor const & col1, OptionColor const & col2)
  {
    return (col1.r == col2.r)
        && (col1.g == col2.g)
        && (col1.b == col2.b)
        && (col1.a == col2.a);
  }
  friend bool operator!= (OptionColor const & col1, OptionColor const & col2)
  {
    return ! (col1 == col2);
  } 
};

class OptionDate
{
public:
  static void parse (OptionDate *, const std::string &);
  OptionDate () {}
  explicit OptionDate (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  static OptionDate interpolate (const OptionDate &, const OptionDate &, const float);
  static OptionDate date_from_t (time_t);
  static time_t tFromDate (const OptionDate &);
  const std::string asString () const;
  const std::string asJSON () const;
  friend std::ostream & operator << (std::ostream &, const OptionDate &);
  friend std::istream & operator >> (std::istream &, OptionDate &);
  friend bool operator== (OptionDate const & d1, OptionDate const & d2)
  {
    return (d1.year   == d2.year  )
        && (d1.month  == d2.month )
        && (d1.day    == d2.day   )
        && (d1.hour   == d2.hour  )
        && (d1.minute == d2.minute)
        && (d1.second == d2.second);
  }
};

class OptionBlock
{
public:
  friend std::ostream & operator << (std::ostream & os, const OptionBlock &) { return os; }
  friend std::istream & operator >> (std::istream & is, OptionBlock &) { return is; }
  friend bool operator== (OptionBlock const & o1, OptionBlock const & o2) { return &o1 == &o2; }
};

namespace OptionsParserDetail 
{

  class optionBase
  {
  public:
    optionBase (const std::string & n, const std::string & d) : name (n), desc (d) {}
    virtual ~optionBase () {}
    virtual int hasArg () const { return 1; }
    virtual void set (const std::string &) 
    { throw std::runtime_error (std::string ("Set method is not defined")); }
    virtual void set () 
    { throw std::runtime_error (std::string ("Set method is not defined")); }
    std::string name;
    std::string desc;
    virtual const bool hasHelp () = 0;
    virtual const std::string type ()  = 0;
    virtual const std::string asString () const = 0;
    virtual const std::string asJSON   () const = 0;
    virtual const std::string asOption () const = 0;
    virtual void clear () = 0;
    virtual bool isEqual (const optionBase *) const = 0;
    bool hidden = false;
  };
 
  template <class T>
  class optionTmpl : public optionBase
  {
  public:
    optionTmpl (const std::string & n, const std::string & d, T * v = nullptr) : optionBase (n, d), value (v) {}
    T * value = nullptr;
    const std::string asString () const { std::ostringstream ss; ss << *value; return std::string (ss.str ()); }
    const std::string asJSON   () const { return OptionsUtil::escape (asString ()); }
    const std::string asOption () const { return name + " " + OptionsUtil::escape (asString ()); }
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
    const bool hasHelp () { return true; }
    const std::string type () { return std::string ("UNKNOWN"); }
    void clear () {}
    int hasArg () const { return 1; }
    bool isEqual (const optionBase * _o) const
    {
      const optionTmpl<T> * o = nullptr;
      try
        {
          o = dynamic_cast<const optionTmpl<T>*>(_o);
	}
      catch (...)
        {
          return false;
        }
      return *(o->value) == *value;
    }
  };

  template <class T>
  class optionTmplList : public optionBase
  {
  public:
    optionTmplList (const std::string & n, const std::string & d, std::vector<T> * v = nullptr) : optionBase (n, d), value (v) {}
    std::vector<T> * value = nullptr;
    const std::string asOption () const 
    { 
      std::string str = name;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); ++it)
        {
          std::ostringstream ss;
          ss << (*it) << " ";
	  str = str + " " + OptionsUtil::escape (std::string (ss.str ()));
	}
      return str;
    }
    const std::string asString () const
    {
      std::ostringstream ss;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); ++it)
        ss << (*it) << " ";
      return std::string (ss.str ());
    }
    const std::string asJSON () const
    {
      std::string json;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); ++it)
        {
          std::ostringstream ss;
          ss << (*it);
          if (it != value->begin ())
            json += ",";
          json += OptionsUtil::escape (std::string (ss.str ()));
        }
      return std::string ("[") + json + std::string ("]");
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
    const bool hasHelp () { return true; }
    const std::string type () { return std::string ("UNKNOWN"); }
    bool isEqual (const optionBase * _o) const 
    {
      const optionTmplList<T> * o = nullptr;
      try
        {
          o = dynamic_cast<const optionTmplList<T>*>(_o);
	}
      catch (...)
        {
          return false;
        }
      return *(o->value) == *value;
    }
  };

  template <> const std::string optionTmpl    <int>                   ::type ();
  template <> const std::string optionTmpl    <float>                 ::type ();
  template <> const std::string optionTmplList<int>                   ::type ();
  template <> const std::string optionTmplList<float>                 ::type ();
  template <> const std::string optionTmpl    <OptionDate>            ::type ();
  template <> const std::string optionTmpl    <OptionBlock>           ::type ();
  template <> const bool        optionTmpl    <OptionBlock>           ::hasHelp ();
  template <> const std::string optionTmpl    <OptionColor>           ::type ();
  template <> const std::string optionTmpl    <OptionScale>           ::type ();
  template <> const std::string optionTmpl    <OptionProjection>      ::type ();
  template <> const std::string optionTmpl    <OptionPaletteName>     ::type ();
  template <> const std::string optionTmpl    <OptionTransformation>  ::type ();
  template <> const std::string optionTmpl    <OptionFieldType>       ::type ();
  template <> const std::string optionTmpl    <OptionPath>            ::type ();
  template <> const std::string optionTmplList<OptionPath>            ::type ();
  template <> const std::string optionTmplList<OptionPath>            ::asString () const;
  template <> const std::string optionTmplList<OptionPath>            ::asJSON   () const;
  template <> const std::string optionTmplList<OptionPath>            ::asOption () const;
  template <> const std::string optionTmpl    <OptionFieldRef>        ::type ();
  template <> const std::string optionTmplList<OptionFieldRef>        ::type ();
  template <> const std::string optionTmplList<OptionFieldRef>        ::asString () const;
  template <> const std::string optionTmplList<OptionFieldRef>        ::asJSON   () const;
  template <> const std::string optionTmplList<OptionFieldRef>        ::asOption () const;
  template <> const std::string optionTmpl    <std::string>           ::type ();
  template <> const std::string optionTmpl    <std::string>           ::asString () const;
  template <> const std::string optionTmpl    <std::string>           ::asJSON   () const;
  template <> const std::string optionTmpl    <std::string>           ::asOption () const;
  template <> const std::string optionTmplList<std::string>           ::type ();
  template <> const std::string optionTmplList<std::string>           ::asString () const;
  template <> const std::string optionTmplList<std::string>           ::asJSON   () const;
  template <> const std::string optionTmplList<std::string>           ::asOption () const;
  template <> const std::string optionTmpl    <bool>                  ::type ();
  template <> void optionTmpl                 <bool>                  ::set ();
  template <> void optionTmplList             <std::string>           ::set (const std::string &);
  template <> void optionTmpl                 <std::string>           ::set (const std::string &);
  template <> void optionTmpl                 <bool>                  ::clear ();
  template <> const std::string optionTmpl    <bool>                  ::asString () const;
  template <> const std::string optionTmpl    <bool>                  ::asJSON   () const;
  template <> const std::string optionTmpl    <bool>                  ::asOption () const;
  template <> int optionTmpl                  <bool>                  ::hasArg () const;

};

class OptionsBase;

class OptionsCallback
{
public:

  class opt
  {
  public:
  virtual void _dummy_ () {}
  };

  virtual void startBlock (const std::string & path, const std::string & name, const std::string & desc, const OptionsCallback::opt * = nullptr)
  {
  }

#define DEF_APPLY(T) \
  virtual void apply (const std::string & path, const std::string & name, OptionsBase *, \
                      const std::string & desc, T * data, const opt * = nullptr) {}
  DEF_APPLY (float);
  DEF_APPLY (bool);
  DEF_APPLY (int);
  DEF_APPLY (std::vector<std::string>);
  DEF_APPLY (std::string);
  DEF_APPLY (std::vector<float>);
  DEF_APPLY (std::vector<int>);
  DEF_APPLY (OptionColor);
  DEF_APPLY (std::vector<OptionColor>);
  DEF_APPLY (OptionScale);
  DEF_APPLY (OptionLongitude);
  DEF_APPLY (OptionLatitude);
  DEF_APPLY (OptionAngle);
  DEF_APPLY (OptionProjection);
  DEF_APPLY (OptionPaletteName);
  DEF_APPLY (OptionTransformation);
  DEF_APPLY (OptionPath);
  DEF_APPLY (OptionFieldType);
  DEF_APPLY (std::vector<OptionPath>);
  DEF_APPLY (OptionFieldRef);
  DEF_APPLY (std::vector<OptionFieldRef>);
  DEF_APPLY (OptionDate);
#undef DEF_APPLY
};


class OptionsParser : public OptionsCallback
{
public:
  virtual const std::string asOption (OptionsParser &);
  bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  void showHelpShort ();
  void showHelpLong ();
  void showJSON ();
  void showPOD ();
  const std::string getHelpShort () const;
  const std::string getPOD  (const std::string &, bool = false, bool = false);
  const std::string getHelp (const std::string &, bool = false, bool = false);
  const std::string getJSON (const std::string &, bool = false, bool = false,
                             glGrib::OptionsParser * = nullptr);
  void getValue (std::vector<std::string> *, const std::string &, 
                 bool = false, bool = false, OptionsParser * = nullptr);
  ~OptionsParser ()
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); ++it)
      delete it->second;
  }
  
  bool seenOption (const std::string &) const;
  void getOptions (std::vector<std::string> * vs)
  {
    for (name2option_t::iterator it = name2option.begin (); 
         it != name2option.end (); ++it)
      vs->push_back (it->first);
  }

  const std::set<std::string> getSeenOptions () const
  {
    return seen;
  }

  const OptionsParserDetail::optionBase * getOption (const std::string & name)
  {
    name2option_t::iterator it = name2option.find (name);
    if (it != name2option.end ())
      return it->second;
    return nullptr;
  }

  class opt : public OptionsCallback::opt
  {
  public:
    bool hidden = false;
  };

private:

  const std::string getonsDoc (const std::string &, bool = false, bool = false);

  std::vector<std::string> ctx;
  std::set<std::string> seen;

  class name2option_t : public std::map<std::string,OptionsParserDetail::optionBase*> 
  {
  public:
    void insert (const std::string name, OptionsParserDetail::optionBase * opt)
    {
      std::map<std::string,OptionsParserDetail::optionBase*>::insert 
        (std::pair<std::string,OptionsParserDetail::optionBase *>(name, opt));
    }
  };

  name2option_t name2option;
  std::vector<std::string> listoptions;

  const std::vector<name2option_t::iterator> filterOptions
     (const std::string &, bool = false, bool = false, 
      glGrib::OptionsParser * = nullptr);

  const std::string getOptName (const std::string & path, const std::string & name)
  {
    return "--" + path + (path == "" ? "" : ".") + name;
  }

  void createOption (const std::string & opt_name, 
                     OptionsParserDetail::optionBase * option, 
                     const OptionsCallback::opt * _o)
  {
    const opt * o = dynamic_cast<const opt*>(_o);
    if (o != nullptr)
      option->hidden = o->hidden;
    name2option.insert (opt_name, option);
    listoptions.push_back (opt_name);
  }

#define DEF_APPLY(T,C) \
  void apply (const std::string & path, const std::string & name,    \
              OptionsBase *, const std::string & desc, T * data,     \
              const OptionsCallback::opt * o = nullptr) override     \
  {                                                                            \
    std::string opt_name = getOptName (path, name);                            \
    createOption (opt_name, new C (opt_name, desc, data), o);                  \
  }

  DEF_APPLY (float                             , OptionsParserDetail::optionTmpl<float> );
  DEF_APPLY (bool                              , OptionsParserDetail::optionTmpl<bool>);
  DEF_APPLY (int                               , OptionsParserDetail::optionTmpl<int>);
  DEF_APPLY (std::vector<std::string>          , OptionsParserDetail::optionTmplList<std::string>);
  DEF_APPLY (std::string                       , OptionsParserDetail::optionTmpl<std::string>);
  DEF_APPLY (std::vector<float>                , OptionsParserDetail::optionTmplList<float>);
  DEF_APPLY (std::vector<int>                  , OptionsParserDetail::optionTmplList<int>);
  DEF_APPLY (OptionColor                       , OptionsParserDetail::optionTmpl<OptionColor>);
  DEF_APPLY (std::vector<OptionColor>          , OptionsParserDetail::optionTmplList<OptionColor>);
  DEF_APPLY (OptionScale                       , OptionsParserDetail::optionTmpl<OptionScale>);
  DEF_APPLY (OptionLongitude                   , OptionsParserDetail::optionTmpl<OptionLongitude>);
  DEF_APPLY (OptionLatitude                    , OptionsParserDetail::optionTmpl<OptionLatitude>);
  DEF_APPLY (OptionAngle                       , OptionsParserDetail::optionTmpl<OptionAngle>);
  DEF_APPLY (OptionFieldType                   , OptionsParserDetail::optionTmpl<OptionFieldType>);
  DEF_APPLY (OptionPath                        , OptionsParserDetail::optionTmpl<OptionPath>);
  DEF_APPLY (OptionProjection                  , OptionsParserDetail::optionTmpl<OptionProjection>);
  DEF_APPLY (OptionPaletteName                 , OptionsParserDetail::optionTmpl<OptionPaletteName>);
  DEF_APPLY (OptionTransformation              , OptionsParserDetail::optionTmpl<OptionTransformation>);
  DEF_APPLY (std::vector<OptionPath>           , OptionsParserDetail::optionTmplList<OptionPath>);
  DEF_APPLY (OptionFieldRef                    , OptionsParserDetail::optionTmpl<OptionFieldRef>);
  DEF_APPLY (std::vector<OptionFieldRef>       , OptionsParserDetail::optionTmplList<OptionFieldRef>);
  DEF_APPLY (OptionDate                        , OptionsParserDetail::optionTmpl<OptionDate>);

#undef DEF_APPLY

  void startBlock (const std::string & path, const std::string & name, const std::string & desc, const OptionsCallback::opt * = nullptr) override;

};

class OptionsBase 
{
public:
  virtual void traverse (const std::string &, OptionsCallback *, 
                         const OptionsCallback::opt * = nullptr) {}
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  virtual bool parse (const char *, const std::set<std::string> * = nullptr);
  virtual const std::string asOption (OptionsBase &);
  friend bool operator== (const OptionsBase &, const OptionsBase &);
};


#define DESC(name, desc) do { cb->apply (p, #name, this, #desc, &name, o); } while (0)
#define DESC_H(name, desc) \
  do { OptionsParser::opt o; o.hidden = true; \
       cb->apply (p, #name, this, #desc, &name, &o); } while (0)

#define INCLUDE_N(name,desc) \
  do { \
    const std::string pp = p + ( p == "" ? "" : ".") + #name; \
    cb->startBlock (pp, #name, #desc, o); \
    name.traverse (pp, cb, o); \
  } while (0)

#define BLOC(name,desc) \
  do { \
    const std::string pp = p + ( p == "" ? "" : ".") + #name; \
    cb->startBlock (pp, #name, #desc, o); \
  } while (0)

#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb, o); } while (0)

#define INCLUDE_H(name) \
  do { OptionsParser::opt o; o.hidden = true; \
       name.traverse (p + ( p == "" ? "" : ".") + #name, cb, &o); } while (0) \

#define DEFINE virtual void traverse (const std::string & p, OptionsCallback * cb, \
                                      const OptionsCallback::opt * o = nullptr)    

class OptionsGeometry : public OptionsBase
{
public:
  DEFINE
  {
    DESC (subgrid.on,         Create subgrid);
    DESC (triangle_strip.on,  Enable use of triangle strip);
    DESC (check.on,           Check geometry);

    BLOC (height,             Height parameters);
    DESC (height.on,          Enable height);
    DESC (height.path,        Height field path);
    DESC (height.scale,       Scale to apply to height field);

    BLOC (frame,              Domain frame);
    DESC (frame.on,           Draw frame around domain);
    DESC (frame.color1,       First frame color);
    DESC (frame.color2,       Second frame color);
    DESC (frame.width,        Frame width);
    DESC (frame.dlon,         Longitude interval);
    DESC (frame.dlat,         Latitude interval);

    DESC (gencoords.on,       Generate coordinates on GPU when possible);

    BLOC (gaussian,           Gaussian geometry options);
    DESC (gaussian.fit.on,    Fit Gaussian latitude retrieval);
    DESC (gaussian.apply_norm_scale.on, Apply norm scaling);
  }
  struct
  {
    bool on = false;
  } subgrid;
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
    OptionFieldRef path;
    float scale = 0.05;
  } height;
  struct 
  {
    bool on = false;
    OptionColor color1 = OptionColor (255, 255, 255);
    OptionColor color2 = OptionColor ( 80,  80,  80);
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
    struct
    {
      bool on = true;
    } apply_norm_scale;
  } gaussian;
};

class OptionsFont : public OptionsBase
{
public:
  OptionsFont (const std::string & b, float s) 
  {
    bitmap.path = b;
    bitmap.scale = s;
  }
  OptionsFont (float s) 
  {
    bitmap.scale  = s;
  }
  OptionsFont () {}
  DEFINE
  {
    DESC (bitmap.path,      Bitmap path);
    DESC (bitmap.scale,     Bitmap scale);
    DESC (color.foreground, Foreground color);
    DESC (color.background, Background color);
  }
  struct 
  {
    OptionPath path = "fonts/08.bmp";
    float scale = 0.05f;
  } bitmap;
  struct
  {
    OptionColor foreground = OptionColor (255, 255, 255, 255);
    OptionColor background = OptionColor (  0,   0,   0,   0);
  } color;
};

class OptionsContour : public OptionsBase
{
public:
  static float defaultMin () { return +std::numeric_limits<float>::max (); }
  static float defaultMax () { return -std::numeric_limits<float>::max (); }
  DEFINE
  {
    DESC (number,        Number of levels);
    DESC (levels,        List of levels);
    DESC (min,           Min level);
    DESC (max,           Max level);
    DESC (widths,        List of widths);
    DESC (patterns,      List of dash patterns);
    DESC (lengths,       List of dash lengths);
    BLOC (labels,        Contour labels);
    DESC (labels.on,     Enable labels);
    INCLUDE (labels.font);
    DESC (labels.distmin, Minimal length in degrees for labelled lines);
    DESC (labels.format,  Format to print labels);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin ();
  float max = defaultMax ();
  std::vector<float> widths;
  std::vector<std::string> patterns;
  std::vector<float> lengths;
  struct
  {
    bool on = false;
    OptionsFont font;
    float distmin = 3.0f;
    std::string format = "%12.2f";
  } labels;
};

class OptionsIsofill : public OptionsBase
{
public:
  static float defaultMin () { return +std::numeric_limits<float>::max (); }
  static float defaultMax () { return -std::numeric_limits<float>::max (); }
  DEFINE
  {
    DESC (number,        Number of levels);
    DESC (levels,        List of levels);
    DESC (min,           Min level);
    DESC (max,           Max level);
  }
  int number = 10;
  std::vector<float> levels;
  float min = defaultMin ();
  float max = defaultMax ();
};

class OptionsStream : public OptionsBase
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

class OptionsVector : public OptionsBase
{
public:
  DEFINE
  {
    DESC (norm.on,         Show norm field);            
    DESC (density,         Vector density);
    DESC (scale,           Vector scale);

    BLOC (arrow,           Arrow parameters);
    DESC (arrow.on,        Show arrows);                
    DESC (arrow.color,     Color for arrows);
    DESC (arrow.head_size, Vector head size);
    DESC (arrow.kind,      Arrow kind);
    DESC (arrow.fill.on,   Fill arrow);
    DESC (arrow.fixed.on,  Fixed sized arrow);
    DESC (arrow.min,       Vector min value);

    BLOC (barb,            Barb parameters);
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
    OptionColor color;
    struct
    {
      bool on = false;
    } fixed;
    float min = 0.0f;
  } arrow;
  struct
  {
    bool on = false;
    OptionColor color;
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

class OptionsPalette : public OptionsBase
{
public:
  static float defaultMin () { return +std::numeric_limits<float>::max (); }
  static float defaultMax () { return -std::numeric_limits<float>::max (); }
  OptionsPalette () {}
  OptionsPalette (const std::string & n) : name (n) {}
  DEFINE
  {
    DESC (name,        Palette name);                              
    DESC (min,         Palette min value);                              
    DESC (max,         Palette max value);                              
    DESC (values,      Palette values);
    DESC (colors,      Palette colors);
    DESC (ncolors,     Number of colors);
    DESC (linear.on,   Linear colorbar display);
    DESC (scale,       Scale for colorbar figures);
    DESC (offset,      Offset for colorbar figures);
    DESC (rainbow.on,  Generate rainbow palette);
    DESC (rainbow.direct.on,  Direct/indirect rainbow);
    DESC (generate.on,        Generate values);
    DESC (generate.levels,    Number of values to generate);
    DESC (fixed.on,           Fixed palette);
  }
  OptionPaletteName name = "default";
  float min = defaultMin ();
  float max = defaultMax ();
  std::vector<float> values;
  std::vector<OptionColor> colors;
  int ncolors = 256;
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
  struct
  {
    bool on = false;
  } fixed;
};

class OptionsScalar : public OptionsBase
{
public:
  DEFINE
  {
    DESC (smooth.on,    Smooth scalar fields);
    DESC (wireframe.on, Display field as wireframe);

    BLOC (points,       Points parameters);
    DESC (points.on,    Display field as points);
    DESC (points.size.value,  Field point size);
    DESC (points.size.variable.on,  Variable field point size);
    DESC (points.size.factor.on,  Apply scale factor to point size);

    DESC (pack.bits,    Number of bytes used to pack field);

    DESC (discrete.on,  Plot as a discrete field);
    DESC (discrete.integer.on,    Values are integers);
    DESC (discrete.missing_color, Color for missing values);

    DESC (light.reverse.on,       Reverse light);
    DESC (light.reverse.b,        Reverse light coefficient);
    DESC (light.reverse.c,        Reverse light coefficient);

    DESC (widen.on, Enable region widening);
    DESC (widen.values, Values for regions);
    DESC (widen.radius, Radius for regions);
    DESC (widen.merge.on, Merge other regions);
    DESC (widen.merge.value, Merge value);

    DESC (mask.on, Enable mask);
    DESC (mask.rand.on, Random mask);
    DESC (mask.frames, Apply mask on this number of frames);
    DESC (mask.x, Mask distribution);
    DESC (mask.y, Mask distribution);
    DESC (mask.path, Path);
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
    struct
    {
      bool on = false;
    } integer;
    OptionColor missing_color = OptionColor (0, 0, 0, 0);
  } discrete;

  struct
  {
    struct
    {
      bool on = false;
      float b = 0.2;
      float c = 15.;
    } reverse;
  } light;

  struct
  {
    bool on = false;
    std::vector<float> values = {0.0f};
    std::vector<int>   radius = {0};
    struct
    {
      bool on = false;
      static float defaultValue () { return +std::numeric_limits<float>::max (); }
      float value = defaultValue ();
    } merge;
  } widen;


  struct
  {
    bool on = false;
    struct
    {
      bool on = false;
    } rand;
    int frames = 100;
    std::vector<float> x = {0.0f, 1.0f};
    std::vector<float> y = {0.0f, 1.0f};
    std::string path;
  } mask;
};

class OptionsMpiview : public OptionsBase
{
public:
  DEFINE
  { 
    DESC (on,    Enable MPI view);
    DESC (path,  Path to MPI distribution field);
    DESC (scale, Displacement scale);
  }

  OptionFieldRef path;
  bool on = false;
  float scale = 0.1f;
};

class OptionsMissing : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,      Enable);
    DESC (value,   Value to be considered as missing);
    DESC (epsilon, Tolerance for missing values);
  }

  bool on = false;
  float value = 0;
  float epsilon = 0;
};

class OptionsVertical : public OptionsBase
{
public:
  DEFINE
  {
    INCLUDE_N (palette, Points palette);
    DESC (lat, List of latitudes);
    DESC (lon, List of longitudes);
    DESC (path, List of GRIB files);
    DESC (scale, Scale);
    DESC (contour.fracdx, Fraction of dx required to keep points);
    DESC (contour.skipmax, Max number of points to skip);
    DESC (wireframe.on, Enable wireframe mode);
    DESC_H (debug.on, Debug);
    DESC (height.constant.on, Enable constant height);
    DESC (height.constant.levels, Levels for constant height);
    DESC (height.uniform.on, Enable uniform height);
    DESC (height.uniform.dz, Uniform height increment);
    DESC (rough.on, Follow grid);
  }

  OptionsPalette palette = OptionsPalette ("none");
  std::vector<float> lat;
  std::vector<float> lon;
  std::vector<OptionFieldRef> path;
  float scale = 1.0f;
  struct
  {
    float fracdx = 0.4f;
    int skipmax = 3;
  } contour;
  struct
  {
    bool on = false;
  } wireframe;
  struct 
  {
    bool on = false;
  } debug;
  struct
  {
    struct
    {
      std::vector<float> levels;
      bool on = false;
    } constant;
    struct
    {
      float dz = 0.05f;
      bool on = false;
    } uniform;
  } height;
  struct
  {
    bool on = false;
  } rough;
};

class OptionsField : public OptionsBase
{
public:
  DEFINE
  {
    DESC (type,                Field type : SCALAR VECTOR STREAM CONTOUR VERTICAL);
    DESC (visible.on,          Field is visible);
    DESC (user_pref.on,        Lookup field settings in database);
    DESC (path,                List of GRIB files);                    
    DESC (scale,               Scales to be applied to fields);        
    DESC (no_value_pointer.on, Do not keep field values in memory);    
    DESC (diff.on,             Show field difference);

    BLOC (hilo,                High & low values);
    DESC (hilo.on,             Display low & high);
    DESC (hilo.radius,         High/low radius in degrees);
    INCLUDE (hilo.font);

    INCLUDE_N (palette, Field palette);

    INCLUDE (scalar);
    INCLUDE (vector);
    INCLUDE (contour);
    INCLUDE (isofill);
    INCLUDE (stream);
    INCLUDE (vertical);

    INCLUDE_N (geometry,   Geometry options);
    INCLUDE_N (mpiview,    MPI distribution parameters);
    DESC (fatal.on,        Fatal error if field fails to be created);

    INCLUDE_N (missing,    Handle some values as missing);

    DESC (slot,            Field slot);
  }
  std::set<std::string> seen;

  struct
  {
    bool on = true;
  } visible;

  struct 
  {
    bool on = false;
    OptionsFont font;
    float radius = 10.0f;
  } hilo;

  OptionFieldType type = "SCALAR";
  struct
  {
    bool on = true;
  } user_pref;
  std::vector<OptionFieldRef> path;
  OptionScale scale = 1.0f;
  struct
  {
    bool on = false;
  } no_value_pointer;
  struct
  {
    bool on = false;
  } diff;
  OptionsPalette palette;
  OptionsScalar scalar;
  OptionsVector vector;
  OptionsContour contour;
  OptionsIsofill isofill;
  OptionsStream stream;
  OptionsVertical vertical;
  bool parseUnseen (const char *);
  OptionsGeometry geometry;
  OptionsMpiview mpiview;
  OptionsMissing missing;

  struct
  {
    bool on = false;
  } fatal;

  float slot = 0.0f;
};


class OptionsTicksSide : public OptionsBase
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

class OptionsTicks : public OptionsBase
{
public:
  DEFINE
  {
    DESC (visible.on,              Ticks are visible);

    BLOC (lines,                   Line options);
    DESC (lines.on,                Display ticks);
    DESC (lines.color,             Tick color);
    DESC (lines.length,            Tick length);
    DESC (lines.width,             Tick width);
    DESC (lines.kind,              Tick kind);

    BLOC (labels,                  Tick labels options);
    DESC (labels.on,               Display tick labels);
    DESC (labels.format,           Format for tick labels);
    INCLUDE (labels.font);

    BLOC (frame,                   Window frame);
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
    bool on = true;
  } visible;

  struct 
  {
    bool on = false;
    OptionsFont font = OptionsFont (0.02f);
    std::string format = "%+06.2f";
  } labels;
  struct
  {
    bool on = false;
    float width = 0.01f;
    OptionColor color = OptionColor (255,   0,   0);
  } frame;
  struct
  {
    bool on = false;
    OptionColor color = OptionColor (255, 255, 255);
    float length = 0.025f;
    float width  = 0.010f;
    int kind     = 0;
  } lines;
  OptionsTicksSide N, S, W, E;
};

class OptionsGrid : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,                Display grid);
    DESC (visible.on,        Grid is visible);
    DESC (resolution,        Grid resolution);
    DESC (interval,          Interval between non dashed lines);
    DESC (dash_length,       Dash length in degrees);
    DESC (points,            Number of points along a parallel);
    DESC (color,             Grid color);
    DESC (scale,             Grid scale);

    BLOC (labels,            Grid labels);
    DESC (labels.on,         Enable labels);
    DESC (labels.lon,        Longitude of latitude labels);
    DESC (labels.lat,        Latitude of longitude labels);
    DESC (labels.angle,      Angle of labels);
    INCLUDE (labels.font);
  }

  struct
  {
    bool on = true;
  } visible;
  int resolution = 9;
  int points = 200;
  int interval = 2;
  float dash_length = 4.0f;
  OptionColor color = OptionColor (0, 255, 0);
  bool on = false;
  OptionScale scale = 1.005;
  struct 
  {
    bool on = false;
    OptionLongitude lon = 0.0f;
    OptionLatitude lat = 0.0f;
    float angle = 0.0f;
    OptionsFont font;
  } labels;
};

class OptionsLandLayer : public OptionsBase
{
public:
  OptionsLandLayer (const std::string & _path, float _scale, const OptionColor & _color)
                           :  path (_path), scale (_scale), color (_color) {}
  DEFINE
  {
    DESC (on,                 Enable);
    DESC (selector,           Shape selection);
    DESC (path,               Path to coastlines);
    DESC (subdivision.angle,  Angle max for subdivision);
    DESC (subdivision.on,     Enable subdivision);
    DESC (scale,              Scale);
    DESC (color,              Land color);
    DESC_H (debug.on,         Debug);
  }
  std::string selector = "";
  OptionPath path      = "coastlines/shp/GSHHS_c_L1.shp";
  OptionScale scale    = 1.0f;
  struct
  {
    float angle        = 1.0f;
    bool on            = true;
  } subdivision;
  OptionColor color = OptionColor ("#ffe2ab");
  struct
  {
    bool on            = false;
  } debug;
  bool on              = true;
};

class OptionsLand : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable land);
    DESC (visible.on,  Land is visible);
    INCLUDE   (layers[0]);
    INCLUDE_H (layers[1]);
    INCLUDE_H (layers[2]);
    INCLUDE_H (layers[3]);
  }
  
  bool on = false;

  struct
  {
    bool on = true;
  } visible;

  std::vector<OptionsLandLayer> layers = 
  {
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L1.shp", 1.000f, OptionColor ("#ffe2ab")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L2.shp", 1.001f, OptionColor ("#0000ff")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L3.shp", 1.002f, OptionColor ("#ffe2ab")),
    OptionsLandLayer ("coastlines/shp/GSHHS_c_L5.shp", 1.000f, OptionColor ("#ffe2ab")) 
  };

};

class OptionsLandscapePosition : public OptionsBase
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

class OptionsLandscape : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,                  Enable landscape);
    DESC (visible.on,          Landscape is visible);
    DESC (projection,          Projection : LONLAT or WEBMERCATOR);
    DESC (flat.on,             Make Earth flat);
    DESC (path,                Path to landscape image in BMP format);
 
    BLOC (grid,                Landscape geometry);
    DESC (grid.path,           Take geometry from this file);
    DESC (grid.number_of_latitudes, Number of latitudes);
    DESC (wireframe.on,        Draw landscape in wireframe mode);
    DESC (scale,               Scale);
    DESC (color,               Color);
    INCLUDE (lonlat.position);
    INCLUDE (geometry);
    DESC (format,              Format = RGB/RGBA);
  }

  struct
  {
    bool on = true;
  } visible;
  std::string projection = "LONLAT";
  std::string path  = "landscape/Whole_world_-_land_and_oceans_08000.png";
  float  orography  = 0.05;
  struct
  {
    std::string path = "";
    int number_of_latitudes  = 500;
  } grid;
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
    OptionsLandscapePosition position;
  } lonlat;
  OptionsGeometry geometry;
  OptionScale scale = 1.0f;
  OptionColor color = OptionColor ("#00000000");
  std::string format = "RGB";
};

class OptionsLines : public OptionsBase
{
public:
  OptionsLines () {}
  OptionsLines (const std::string & p, const std::string & f) : path (p), format (f) {}
  DEFINE
  {
    DESC (visible.on,         Lines are visible);
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
  struct
  {
    bool on = true;
  } visible;
  std::string selector;
  std::string path;
  std::string format = "gshhg";
  OptionColor color;
  OptionScale scale = 1.005;
  float latmin = 0.0f, latmax = 0.0f, lonmin = 0.0f, lonmax = 0.0f;
};

class OptionsOffscreen : public OptionsBase
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

class OptionsRender : public OptionsBase
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
    DESC (opengl.version,        OpenGL version);
    INCLUDE (offscreen);
    DESC (info.on,               Show hardware info);
    DESC_H (fixlandscape.on,     Fix landscape position);

    BLOC (position,              Window position);
    DESC (position.x,            Window x position);
    DESC (position.y,            Window y position);

    BLOC (fullscreen,            Fullscreen window);
    DESC (fullscreen.on,         Window in fullscreen mode);
    DESC (fullscreen.x.on,       Window in fullscreen mode in X direction);
    DESC (fullscreen.y.on,       Window in fullscreen mode in Y direction);
#ifdef GLGRIB_USE_EGL
#ifdef GLGRIB_USE_GBM
    DESC (egl.gbm.on,            Enable gbm);
    DESC (egl.gbm.path,          Path to gbm device);
#else
    DESC (egl.device,            Device rank);
#endif
#endif
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
  std::string  title  = "";
  struct
  {
    bool on = false;
  } debug;
  struct
  {
    float version = 4.3;
  } opengl;
  OptionsOffscreen offscreen;
  struct
  {
    bool on = false;
  } fixlandscape;
  struct
  {
    bool on = false;
  } info;
  struct
  {
    bool on = false;
    struct
    {
      bool on = false;
    } x;
    struct
    {
      bool on = false;
    } y;
  } fullscreen;

#ifdef GLGRIB_USE_EGL
  struct
  {
#ifdef GLGRIB_USE_GBM
    struct
    {
      bool on = false;
      OptionPath path = "/dev/dri/renderD128";
    } gbm;
#else
    int device = -1;
#endif
  } egl;
#endif
};

class OptionsLight : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,                Enable light);
    DESC (date_from_grib.on, Calculate light position from GRIB date);
    DESC (lon,               Light longitude);
    DESC (lat,               Light latitude);
    DESC (date,              Date for sunlight position);
    DESC (night,             Fraction of light during for night);
   
    BLOC (rotate,            Control light rotation);
    DESC (rotate.on,         Make sunlight move);
    DESC (rotate.rate,       Rate of rotation : angle/frame);
  }
  OptionDate date;
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
  OptionLongitude lon  = 0.0f;
  OptionLatitude  lat  = 0.0f;
  float  night = 0.1f;
};

class OptionsPosition : public OptionsBase
{
public:
  DEFINE
  {
    DESC (lon,            Longitude);
    DESC (lat,            Latitude);
    DESC (fov,            Field of view);
  }
  OptionLongitude  lon  = 0.0f;
  OptionLatitude   lat  = 0.0f;
  float  fov  = 0.0f;
};

class OptionsTravelling : public OptionsBase
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
  OptionsPosition pos1;
  OptionsPosition pos2;
};

class OptionsInterpolation : public OptionsBase
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

class OptionsImage : public OptionsBase
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
  OptionPath path;
  float x0 = 0.0, x1 = 1.0, y0 = 0.0, y1 = 1.0;
  std::string align;
};

class OptionsText : public OptionsBase
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
  OptionsFont font;
};

class OptionsDate : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display date);
    INCLUDE (font);
  }
  bool on = false;
  OptionsFont font;
};

class OptionsTitle : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Enable title);
    DESC (x,  Coordinates);
    DESC (y,  Coordinates);
    DESC (a,  Alignment);
    INCLUDE (font);
    DESC (text, Title);
  }
  bool on = false;
  float x = 0.;
  float y = 1.;
  std::string a = "NW";
  std::string text = "";
  OptionsFont font;
};

class OptionsScene : public OptionsBase
{
public:
  DEFINE
  {
    DESC (lon_at_hour,         Set longitude at solar time);

    BLOC (rotate_earth,        Earth rotation control);
    DESC (rotate_earth.on,     Make earth rotate);
    DESC (rotate_earth.rate,   Rate of rotation : angle/frame);

    INCLUDE_N (light,          Light options);
    INCLUDE_N (travelling,     Travelling options);
    INCLUDE_N (interpolation,  Interpolation options);
    INCLUDE_N (text,           Text options);
    INCLUDE_N (image,          Image options);
    INCLUDE_N (date,           Date options);
    INCLUDE_N (title,          Title options);

    DESC (select.field_rank, Rank of field to select);

    DESC (center.on, Center on first field);
    DESC (center.gridpoint, Rank of grid point to set focus on);
  }
  struct
  {
    bool on  = false;
    float rate = 1.0f;
  } rotate_earth;
  float   lon_at_hour = -1.0f;
  OptionsLight light;  
  OptionsTravelling travelling;
  OptionsInterpolation interpolation;
  OptionsDate date;
  OptionsText text;
  OptionsImage image;
  OptionsTitle title;
  struct
  {
    int field_rank = 0;
  } select;
  struct
  {
    bool on = false;
    int gridpoint = -1;
  } center;
};

class OptionsClip : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on,            Enable Mercator and lat/lon clipping);
    DESC (dlon,          Amount of longitude to clip);
    DESC (dlat,          Amount of latitude to clip);
    DESC (xmin,          Min viewport x coordinate);
    DESC (xmax,          Max viewport x coordinate);
    DESC (ymin,          Min viewport y coordinate);
    DESC (ymax,          Max viewport y coordinate);
  }
  float dlon = 10.0f;
  float dlat =  5.0f;
  float xmin = 0.0f, xmax = 1.0f, ymin = 0.0f, ymax = 1.0f;
  bool on = true;
};

class OptionsView : public OptionsBase
{
public:
  DEFINE
  {
    DESC (projection,         Mercator XYZ latlon polar_north polar_south);
    DESC (transformation,     Perspective or orthographic);
    DESC (lon,                Camera longitude);
    DESC (lat,                Camera latitude);
    DESC (roll,               Camera roll);
    DESC (pitch,              Camera pitch);
    DESC (yaw,                Camera yaw);
    DESC (fov,                Camera field of view);
    DESC (distance,           Camera distance);
    DESC (center.on,          Center view);

    INCLUDE_N (clip,          Clipping options);

    BLOC (zoom,               Zoom options);
    DESC (zoom.on,            Enable zoom with Schmidt transform);
    DESC (zoom.lon,           Longitude of zoom);
    DESC (zoom.lat,           Latitude of zoom);
    DESC (zoom.stretch,       Stretching factor);

    DESC (trans.on,           Enable transformation matrix);
    DESC (trans.matrix,       Transformation matrix values);
  }
  OptionProjection projection = "XYZ";
  OptionTransformation transformation = "PERSPECTIVE";
  float  distance  = 6.0; 
  OptionLatitude  lat       = 0.0; 
  OptionLongitude lon       = 0.0; 
  OptionAngle     roll      = 0.0;
  OptionAngle     pitch     = 0.0;
  OptionAngle     yaw       = 0.0;
  OptionAngle     fov       = 20.;
  struct
  {
    bool on = true;
  } center;
  struct
  {
    bool on = false;
    OptionLongitude lon = 2.0f;
    OptionLatitude  lat = 46.7f;
    float stretch = 0.5f;
  } zoom;
  struct
  {
    bool on = false;
    std::vector<float> matrix;
  } trans;
  OptionsClip clip;
};


class OptionsColorbar : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Activate colorbar);
    INCLUDE_N (font,     Font options);
 
    BLOC (levels,        Colorbar levels);
    DESC (levels.number, Colorbar number of levels);
    DESC (levels.values, Colorbar level values);
    DESC (format, Format (sprintf) use to display numbers);
   
    BLOC (position,      Colorbar position);
    DESC (position.xmin, Colorbar xmin);
    DESC (position.xmax, Colorbar xmax); 
    DESC (position.ymin, Colorbar ymin); 
    DESC (position.ymax, Colorbar ymax);
  }
  bool on = false;
  OptionsFont font = OptionsFont ("fonts/16.bmp", 0.02f);
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

class OptionsMapscale : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Activate mapscale);

    INCLUDE_N (font, Font options);

    BLOC (position,      Mapscale position);
    DESC (position.xmin, Mapscale xmin);
    DESC (position.xmax, Mapscale xmax); 
    DESC (position.ymin, Mapscale ymin); 
    DESC (position.ymax, Mapscale ymax);

    DESC (color1, First color);
    DESC (color2, Second color);
  }
  bool on = false;
  OptionsFont font = OptionsFont ("fonts/16.bmp", 0.02f);
  OptionColor color1 = OptionColor (255, 255, 255);
  OptionColor color2 = OptionColor ( 80,  80,  80);
  struct position
  {
    float xmin = 0.05;
    float xmax = 0.25; 
    float ymin = 0.05; 
    float ymax = 0.07;
  } position;
};

class OptionsRivers : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display rivers);
    INCLUDE (lines);
  }
  bool on = false;
  OptionsLines lines = OptionsLines ("coastlines/gshhg/WDBII_bin/wdb_rivers_f.b", "gshhg");
};

class OptionsBorder : public OptionsBase
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
  OptionsLines lines = OptionsLines ("coastlines/gshhg/WDBII_bin/wdb_borders_f.b", "gshhg");
};

class OptionsPoints : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (visible.on,  Points are visible);
    DESC (scale, Scale);

    BLOC (size,             Size options);
    DESC (size.variable.on, Enable variable point size);
    DESC (size.value, Point size);
 
    INCLUDE_N (palette,     Points palette);
    DESC (color, Point color);
  }
  struct
  {
    bool on = true;
  } visible;
  OptionsPalette palette = OptionsPalette ("none");
  OptionColor color;
  OptionScale scale = 1.0f;
  struct
  {
    float value = 1.0f;
    struct
    {
       bool on = false;
    } variable;
  } size;
};

class OptionsGeoPoints : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (on, Display geopoints);
    DESC (time.on, Make points appear with time);
    DESC (time.rate, Number of points to add at each new frame);
    INCLUDE (points);
    DESC (path, Path);
    DESC (lon, Longitude field name);
    DESC (lat, Latitude field name);
    DESC (val, Value field name);
  }
  bool on = false;
  struct
  {
    bool on = false;
    int rate = 1;
  } time;
  OptionsPoints points;
  OptionPath path;
  std::string lon, lat, val;
};

class OptionsCities : public OptionsBase
{
public:
  DEFINE 
  {
    DESC (on, Display cities);
    INCLUDE (points);

    BLOC (labels,    City labels);
    DESC (labels.on, Enable city names display);
    INCLUDE (labels.font);
  }
  bool on = false;
  OptionsPoints points;
  struct
  {
    OptionsFont font;
    bool on = false;
  } labels;
};

class OptionsCoast : public OptionsBase
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
  OptionsLines lines = OptionsLines ("coastlines/gshhg/GSHHS_bin/gshhs_h.b", "gshhg");
};

class OptionsDepartements : public OptionsBase
{
public:
  DEFINE
  {
    DESC (on, Display departements);
    INCLUDE (lines);
  }
  bool on = false;
  OptionsLines lines = OptionsLines ("coastlines/departements/departements-20180101.shp", "shapeline");
};

class OptionsShell : public OptionsBase
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

class Options : public OptionsBase
{
public:
  DEFINE
  {
    INCLUDE   (field[0]); INCLUDE_H (field[1]); 
    INCLUDE_H (field[2]); INCLUDE_H (field[3]); 
    INCLUDE_H (field[4]); INCLUDE_H (field[5]); 
    INCLUDE_H (field[6]); INCLUDE_H (field[7]); 
    INCLUDE_H (field[8]); INCLUDE_H (field[9]); 

    INCLUDE   (geopoints[0]); INCLUDE_H (geopoints[1]); 
    INCLUDE   (geopoints[2]); INCLUDE_H (geopoints[3]); 
    INCLUDE   (geopoints[4]); INCLUDE_H (geopoints[5]); 
    INCLUDE   (geopoints[6]); INCLUDE_H (geopoints[7]); 
    INCLUDE   (geopoints[8]); INCLUDE_H (geopoints[9]); 

    INCLUDE (coast);
    INCLUDE (cities);
    INCLUDE (border);
    INCLUDE (rivers);
    INCLUDE (render);
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
    DESC (review.on, Enable review mode);
    DESC (review.path, File to review);
    DESC (diff.on, Enable difference mode);
    DESC (diff.path, Files to show in diff mode);
  }
  struct
  {
    bool on = false;
    OptionPath path;
  } review;
  struct
  {
    std::vector<std::string> path;
    bool on = false;
  } diff;
  std::vector<OptionsField> field = {OptionsField (), OptionsField (), OptionsField (), OptionsField (), OptionsField (),
                                     OptionsField (), OptionsField (), OptionsField (), OptionsField (), OptionsField ()};
  OptionsCoast coast;
  OptionsCities cities;
  std::vector<OptionsGeoPoints> geopoints = 
    {OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints (),
     OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints (), OptionsGeoPoints ()};
  OptionsBorder border;
  OptionsRivers rivers;
  OptionsColorbar colorbar;
  OptionsMapscale mapscale;
  OptionsDepartements departements;
  OptionsRender render;
  OptionsLandscape landscape;
  OptionsGrid grid;
  OptionsTicks ticks;
  OptionsScene scene;
  OptionsView view;
  OptionsFont font;
  OptionsShell shell;
  OptionsLand land;
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
};

#undef DESC
#undef INCLUDE
#undef INCLUDE_H

}
