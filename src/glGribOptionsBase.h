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
#include <string.h>
#include <time.h>
#include <string.h>


namespace glGrib
{

namespace OptionsUtil
{
  std::string nextToken (std::string *);
  std::string escape (const std::string &);
};

class OptionColor
{
public:
  static OptionColor colorByName (const std::string &);
  static OptionColor colorByHexa (const std::string &);
  static void parse (OptionColor *, const std::string &);

  OptionColor () {}
  OptionColor (int _r, int _g, int _b, int _a = 255) : r (_r), g (_g), b (_b), a (_a) {}
  OptionColor (const std::string &);

  int r = 255, g = 255, b = 255, a = 255;
  std::string asString () const 
  {
    char str[32]; 
    sprintf (str, "#%2.2x%2.2x%2.2x%2.2x", r, g, b, a); 
    return std::string (str); 
  }
  std::string asJSON () const 
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
  OptionDate (int _year, int _month, int _day, int _hour, int _minute, int _second) : 
    year (_year), month (_month), day (_day), hour (_hour), minute (_minute), second (_second) {}
  long int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
  static OptionDate interpolate (const OptionDate &, const OptionDate &, const float);
  static OptionDate date_from_t (time_t);
  static time_t tFromDate (const OptionDate &);
  std::string asString () const;
  std::string asJSON () const;
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
    virtual std::string type ()  = 0;
    virtual std::string asString () const = 0;
    virtual std::string asJSON   () const = 0;
    virtual std::string asOption () const = 0;
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
    std::string asString () const { std::ostringstream ss; ss << *value; return std::string (ss.str ()); }
    std::string asJSON   () const { return OptionsUtil::escape (asString ()); }
    std::string asOption () const { return name + " " + OptionsUtil::escape (asString ()); }
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
    std::string asOption () const 
    { 
      std::string str = name;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); it++)
        {
          std::ostringstream ss;
          ss << (*it) << " ";
	  str = str + " " + OptionsUtil::escape (std::string (ss.str ()));
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
    std::string asJSON () const
    {
      std::string json;
      for (typename std::vector<T>::iterator it = value->begin(); it != value->end (); it++)
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
    std::string type () { return std::string ("UNKNOWN"); }
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

  template <> std::string optionTmpl     <int>               ::type ();
  template <> std::string optionTmpl     <float>             ::type ();
  template <> std::string optionTmplList<int>                ::type ();
  template <> std::string optionTmplList<float>              ::type ();
  template <> std::string optionTmpl     <OptionDate> ::type ();
  template <> std::string optionTmpl     <OptionColor>::type ();
  template <> std::string optionTmpl     <std::string>       ::type ();
  template <> std::string optionTmpl     <std::string>       ::asString () const;
  template <> std::string optionTmpl     <std::string>       ::asJSON   () const;
  template <> std::string optionTmpl     <std::string>       ::asOption () const;
  template <> std::string optionTmplList<OptionColor>        ::type ();
  template <> std::string optionTmplList<std::string>        ::type ();
  template <> std::string optionTmplList<std::string>        ::asString () const;
  template <> std::string optionTmplList<std::string>        ::asJSON   () const;
  template <> std::string optionTmplList<std::string>        ::asOption () const;
  template <> std::string optionTmpl     <bool>              ::type ();
  template <> void optionTmpl            <bool>              ::set ();
  template <> void optionTmplList        <std::string>       ::set (const std::string &);
  template <> void optionTmpl            <std::string>       ::set (const std::string &);
  template <> void optionTmpl            <bool>              ::clear ();
  template <> std::string optionTmpl     <bool>              ::asString () const;
  template <> std::string optionTmpl     <bool>              ::asJSON   () const;
  template <> std::string optionTmpl     <bool>              ::asOption () const;
  template <> int optionTmpl             <bool>              ::hasArg () const;

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
  DEF_APPLY (OptionDate);
#undef DEF_APPLY
};


class OptionsParser : public OptionsCallback
{
public:
  virtual std::string asOption (OptionsParser &);
  bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  void showHelpShort ();
  void showHelpLong ();
  void showJSON ();
  std::string getHelp (const std::string &, bool = false);
  std::string getJSON (const std::string &, bool = false, 
                       glGrib::OptionsParser * = nullptr);
  void getValue (std::vector<std::string> *, const std::string &, 
                 bool = false, OptionsParser * = nullptr);
  ~OptionsParser ()
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

  std::vector<name2option_t::iterator> filterOptions
     (const std::string &, bool = false, glGrib::OptionsParser * = nullptr);

  std::string getOptName (const std::string & path, const std::string & name)
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
  }

#define DEF_APPLY(T,C) \
  void apply (const std::string & path, const std::string & name,              \
              OptionsBase *, const std::string & desc, T * data,               \
              const OptionsCallback::opt * o = nullptr)                        \
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
  DEF_APPLY (OptionDate                        , OptionsParserDetail::optionTmpl<OptionDate>);

#undef DEF_APPLY

};

class OptionsBase 
{
public:
  typedef std::vector<std::string> string_list;
  typedef std::vector<float> float_list;
  typedef std::string string;
  virtual void traverse (const std::string &, OptionsCallback *, 
                         const OptionsCallback::opt * = nullptr) {}
  virtual bool parse (int, const char * [], const std::set<std::string> * = nullptr);
  virtual bool parse (const char *, const std::set<std::string> * = nullptr);
  virtual std::string asOption (OptionsBase &);
  friend bool operator== (const OptionsBase &, const OptionsBase &);
};


#define DESC(name, desc) do { cb->apply (p, #name, this, #desc, &name, o); } while (0)
#define DESC_H(name, desc) \
  do { OptionsParser::opt o; o.hidden = true; \
       cb->apply (p, #name, this, #desc, &name, &o); } while (0)

#define INCLUDE(name) do { name.traverse (p + ( p == "" ? "" : ".") + #name, cb, o); } while (0)

#define INCLUDE_H(name) \
  do { OptionsParser::opt o; o.hidden = true; \
       name.traverse (p + ( p == "" ? "" : ".") + #name, cb, &o); } while (0) \

#define DEFINE virtual void traverse (const std::string & p, OptionsCallback * cb, \
                                      const OptionsCallback::opt * o = nullptr)    

}
