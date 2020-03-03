#pragma once

#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdexcept>

namespace glGrib
{

class DBase
{
public:

  static void _read (void * ptr, int n, FILE * fp)
  {
    if (fread (ptr, n, 1, fp) != 1)
      throw std::runtime_error (std::string ("Cannot read data from dbase file"));
  }

  class header_t
  { 
  public:
  
     uint8_t   version;        /* 03 for dbIII and 83 for dbIII w/memo file */
     uint8_t   l_update[3];    /* yymmdd for last update*/
     uint32_t  count;          
     uint16_t  length;         
     uint16_t  record_length;  
     uint8_t   padding[20];
  
     void read (FILE * fp)
     {
       _read (&version,         1, fp);
       _read (l_update,         3, fp);
       _read (&count,           4, fp);
       _read (&length,          2, fp);
       _read (&record_length,   2, fp);
       _read (padding,         20, fp);
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
    
     void read (FILE * fp)
     {
       _read (name,       11, fp);
       _read (&type,       1, fp);
       _read (&data_ptr,   4, fp);
       _read (&length,     1, fp);
       _read (&dec_point,  1, fp);
       _read (padding,    14, fp);
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
    close ();
  }

  void close ()
  {
    if (fp != nullptr)
      fclose (fp);
    count = 0;
    fp = nullptr;
  }

  header_t header;
  std::vector<field_t> fields;

  bool opened () const 
  {
    return fp != nullptr;
  }

private:
  map_t map;
  FILE * fp = nullptr;
  int count = 0;
};
 

}
