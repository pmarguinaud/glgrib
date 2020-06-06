#pragma once

#include <string>
#include <vector>
#include <eccodes.h>

namespace glGrib
{

class Container
{
public:
  static Container * create (const std::string &, bool = false);
  static void remove (Container *);
  static void clear ();
  Container (const std::string & _file) : file (_file) {}
  virtual ~Container () {}
  virtual codes_handle * getHandleByExt (const std::string &) = 0;
  virtual bool hasExt (const std::string &) const = 0;
  const std::string & getFile () const { return file; }
  virtual void buildIndex () = 0;
  virtual const std::string & getNextExt (const std::string &) = 0;
  virtual const std::string & getPrevExt (const std::string &) = 0;
protected:
  class _iterator 
  {
  public:
    virtual void incr () = 0;
    virtual const std::string & str () = 0;
    virtual bool isEqual (const _iterator *) const = 0;
    virtual ~_iterator () {}
    virtual bool last () const = 0;
  };
public:
  class iterator
  {
  public:
    iterator () {}
    iterator (_iterator * _it) : it (_it) {}
    iterator & operator++ ()
    {
      it->incr ();
      return *this;
    }
    virtual ~iterator ()
    {
      delete it;
    }
    const std::string & operator* ()
    {    
      return it->str ();
    }    
    bool operator!= (const iterator & rhs) const
    {    
      return ! operator== (rhs);
    }    
    bool operator== (const iterator & rhs) const
    {    
      // Both at end
      if ((it == nullptr) && (rhs.it == nullptr))
        return true;

      // At end or not
      if (rhs.it == nullptr)
        return it->last ();

      // At end ?
      if (it == nullptr)
        return rhs.it->last ();

      return it->isEqual (rhs.it);
    }    
  private:
    _iterator * it = nullptr;
  };
  virtual iterator begin () = 0;
  virtual iterator end () = 0;
private:
  std::string file;
};


}
