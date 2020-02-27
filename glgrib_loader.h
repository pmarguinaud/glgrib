#pragma once

#include <string>
#include <list>
#include <memory>

#include <eccodes.h>
#include "glgrib_field_metadata.h"
#include "glgrib_field_float_buffer.h"
#include "glgrib_handle.h"
#include "glgrib_geometry.h"
#include "glgrib_options.h"

class glgrib_loader 
{
public:

  static void uv2nd (const_glgrib_geometry_ptr,
                     const glgrib_field_float_buffer_ptr, 
                     const glgrib_field_float_buffer_ptr,
                     glgrib_field_float_buffer_ptr &, 
                     glgrib_field_float_buffer_ptr &,
                     const glgrib_field_metadata &, 
                     const glgrib_field_metadata &,
                     glgrib_field_metadata &, 
                     glgrib_field_metadata &);

  glgrib_handle_ptr handle_from_file (const std::string &);
  void load (glgrib_field_float_buffer_ptr *, const std::vector<std::string> &, const glgrib_options_geometry &, float, glgrib_field_metadata *, int = 1, int = 0, bool diff = false);
  void load (glgrib_field_float_buffer_ptr *, const std::string &, const glgrib_options_geometry &, glgrib_field_metadata *);
  void setSize (int _size) { size = _size; }
private:
  int size = 0;

  class entry_t
  {
  public:
    std::string file;
    glgrib_handle_ptr ghp;
  };

  typedef std::list<entry_t> cache_t;
  cache_t cache;
};

class glgrib_container
{
public:
  static glgrib_container * create (const std::string &, bool = false);
  static void clear ();
  glgrib_container (const std::string & _file) : file (_file) {}
  virtual codes_handle * getHandleByExt (const std::string &) = 0;
  const std::string getFile () const { return file; }
  virtual void buildIndex () = 0;
protected:
  class _iterator 
  {
  public:
    virtual void incr () = 0;
    virtual const std::string & str () = 0;
    virtual bool isEqual (const _iterator *) const = 0;
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
    ~iterator ()
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

