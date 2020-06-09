#pragma once

#include <map>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

namespace glGrib
{

class DBase
{
private:
  static void _read (void * ptr, int n, std::ifstream & fh)
  {
    if (! fh.read (reinterpret_cast<char*> (ptr), n))
      throw std::runtime_error (std::string ("Cannot read data from dbase file"));
  }

public:
  
  class header_t
  { 
  public:
  
     uint8_t   version;        /* 03 for dbIII and 83 for dbIII w/memo file */
     uint8_t   l_update[3];    /* yymmdd for last update*/
     uint32_t  count;          
     uint16_t  length;         
     uint16_t  record_length;  
     uint8_t   padding[20];
  
     void read (std::ifstream & fh)
     {
       _read (&version,         1, fh);
       _read (l_update,         3, fh);
       _read (&count,           4, fh);
       _read (&length,          2, fh);
       _read (&record_length,   2, fh);
       _read (padding,         20, fh);
     }
  };
  
  class field_t
  {
  public:
     uint8_t   name[11];                                  
     uint8_t   type;                                      
     uint8_t   *data_ptr;                                  
     uint8_t   length;                                    
     uint8_t   dec_point;                                
     uint8_t   padding[14];
    
     void read (std::ifstream & fh)
     {
       _read (name,       11, fh);
       _read (&type,       1, fh);
       _read (&data_ptr,   4, fh);
       _read (&length,     1, fh);
       _read (&dec_point,  1, fh);
       _read (padding,    14, fh);
     }
  };

  class map_t : public std::map<std::string,int>
  {
  public:
    void add (const std::string & name, int i)
    {
      insert (std::pair<std::string,int>(name, i));
    }
  };

  class record_t : public std::vector<std::string>
  {
  public:
    const std::string & get (const std::string & key)
    {
      return (*this)[map->at (key)];
    }
  private:
    map_t * map = nullptr;
    friend class DBase;
  };

  void convert2sqlite (const std::string &);

  bool open (const std::string &);

  bool read (record_t *);

  ~DBase ()
  {
  }

  void close ()
  {
    if (fh)
      fh.close ();
    count = 0;
  }

  header_t header;
  std::vector<field_t> fields;

  bool opened () const 
  {
    return fh.is_open ();
  }

private:
  map_t map;
  std::ifstream fh;
  int count = 0;
};
 

}
