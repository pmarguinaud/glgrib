#include "glgrib_shapelib.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <shapefil.h>


#include <stdexcept>
#include <iostream>
#include <map>


#include <stdint.h>

class dbase
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
       _read(name,       11, fp);
       _read(&type,       1, fp);
       _read(&data_ptr,   4, fp);
       _read(&length,     1, fp);
       _read(&dec_point,  1, fp);
       _read(padding,    14, fp);
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
    map_t * map = NULL;
    friend class dbase;
  };

  static std::string trim (const std::string & str)
  {
    int i, j;
    for (i = 0; i < str.length (); i++)
      if (str[i] != ' ')
        break;
    for (j = str.length ()-1; j >= 0; j--)
      if (str[j] != ' ')
        break;
    if (i > j)
      return std::string ("");
    return str.substr (i, j-i+1);
  }

  bool open (const std::string & path)
  {
    fp = fopen ((path + ".dbf").c_str (), "r");
    if (fp == NULL)
      return false;
    header.read (fp);
    int n = (header.length - 1)/32 - 1;

    fields.resize (n);

    for (int i = 0; i < n; i++)
      {
        fields[i].read (fp);
        std::string name = std::string ((char *)fields[i].name);
        map.add (trim (name), i);
      }

    fseek (fp, header.length, SEEK_SET);

    return true;
  }

  bool read (record_t * record)
  {
    record->clear ();
    if (count >= header.count)
      return false;

    count++;
    record->map = &map;

    char tmp[header.record_length], * ptr = &tmp[0];
    _read (tmp, header.record_length, fp);
    for (int i = 0; i < fields.size (); i++)
      {
        std::string str (ptr, fields[i].length);
        record->push_back (trim (str));
        ptr += fields[i].length;
      }
    return true;
  }

  ~dbase ()
  {
    if (fp != NULL)
      fclose (fp);
    fp = NULL;
  }

  header_t header;
  std::vector<field_t> fields;

  bool opened () const 
  {
    return fp != NULL;
  }

private:
  map_t map;
  FILE * fp = NULL;
  int count = 0;
};
 
void glgrib_shapelib::read (const std::string & path, int * numberOfPoints, 
                            unsigned int * numberOfLines, std::vector<float> * xyz,
                            std::vector<unsigned int> * ind, 
                            const std::vector<std::string> & hash)
{
  const float deg2rad = M_PI / 180.0f;
  dbase d;

  d.open (path);
  dbase::record_t record;

  SHPHandle fp = SHPOpen (path.c_str (), "r");

  if (fp == NULL)
    throw std::runtime_error (std::string ("Cannot open Shapefile data : ") + path);



  double minb[4], maxb[4];
  int ntype, nentities;
  SHPGetInfo (fp, &nentities, &ntype, minb, maxb);

  *numberOfLines = 0;
  *numberOfPoints = 0;

  int count = -1;
  for (int i = 0; i < nentities; i++)
    {

      SHPObject * shape = SHPReadObject(fp, i);

      if (shape == NULL)
        throw std::runtime_error (std::string ("Cannot read Shapefile data : ") + path);
      
      bool match = false;

      if (d.opened () && (hash.size () > 0))
        {
          dbase::record_t record;
          d.read (&record);

          for (int j = 0; j < (hash.size () / 2); j++)
            {
              const std::string & key = hash[2*j+0], 
                                & val = hash[2*j+1];
              match = match || (record.get (key) == val);
            }
        }
      else
        {
          match = true;
        }

      if (match)
      for (int j = 0, ipart = 0; j < shape->nVertices; j++)
        {
          const char * type;
          bool newpart = false;

          if (j == 0 && shape->nParts > 0)
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }
          
          if ((ipart < shape->nParts) && (shape->panPartStart[ipart] == j))
            {
              type = SHPPartTypeName (shape->panPartType[ipart]);
              ipart++;
              newpart = true;
            }


          float lon = shape->padfX[j] * deg2rad; 
          float lat = shape->padfY[j] * deg2rad;

          float coslon = cos (lon);
          float sinlon = sin (lon);
          float coslat = cos (lat);
          float sinlat = sin (lat);

          xyz->push_back (coslon * coslat);
          xyz->push_back (sinlon * coslat);
          xyz->push_back (         sinlat);

          (*numberOfPoints)++;

          if (newpart)
            {
              count++;
            }
          else
            {
              ind->push_back (count); count++;
              ind->push_back (count);
              (*numberOfLines)++;
            }


        }
    

      SHPDestroyObject (shape);


    }

  SHPClose (fp);

}

