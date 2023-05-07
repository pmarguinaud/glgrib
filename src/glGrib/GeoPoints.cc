#include "glGrib/GeoPoints.h"
#include "glGrib/Trigonometry.h"
#include "glGrib/Resolve.h"
#include "glGrib/SQLite.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>

#include <netcdf>
#include <eccodes.h>
#include <cstdio>

namespace glGrib
{

namespace
{

const std::vector<float> readNC (const netCDF::NcFile & nc, const std::string & name)
{
  netCDF::NcVar var = nc.getVar (name);

  if (var.isNull ()) 
    throw std::runtime_error (std::string ("Cannot read ") + name);
    
  auto dims = var.getDims ();

  if (dims.size () != 1)
    throw std::runtime_error (std::string ("Variable ") 
                            + name + std::string (" is not a vector"));

  std::vector<float> v (dims[0].getSize ());

  var.getVar (&v[0]);
  
  return v;
}


class BUFRhandle
{
public:
  BUFRhandle ()
  {
  }
  BUFRhandle (BUFRhandle && bh) // So that we can pack BUFRhandle objects in std::vector
  {
    packed = bh.packed;
    h = bh.h;
    bh.h = nullptr;
  }
  void readFromFH (FILE * fh)
  {
    int err;
    h = codes_handle_new_from_file (nullptr, fh, PRODUCT_BUFR, &err);
  }
  ~BUFRhandle ()
  {
    clear ();
  }
  void clear ()
  {
    if (h != nullptr)
      {
        if (codes_handle_delete (h))
          throw std::runtime_error (std::string ("BUFR error"));
      }
    h = nullptr;
  }
  bool isValid () const
  {
    return h != nullptr;
  }
  void get (const std::string & name, std::vector<float> & v)
  {
    unpack ();
    size_t size;
    if (codes_get_size (h, name.c_str (), &size))
      throw std::runtime_error (std::string ("BUFR error"));
    std::vector<double> t (size);
    if (codes_get_double_array (h, name.c_str (), &t[0], &size))
      throw std::runtime_error (std::string ("BUFR error"));
    v.reserve (v.size () + size);
    for (size_t i = 0; i < size; i++)
      v.push_back (t[i]);
  }
  void unpack ()
  {
    if (packed && (h != nullptr))
      {
        if (codes_set_long (h, "unpack", 1))
          throw std::runtime_error (std::string ("BUFR error"));
	packed = false;
      }
  }
private:
  bool packed = true;
  codes_handle * h = nullptr;
};

void readBUFR (FILE * fh, 
               const std::string & lon_n, const std::string & lat_n, const std::string & val_n,
               std::vector<float> & lon, std::vector<float> & lat, std::vector<float> & val)
{
  int mess = 0;

  while (1)
    {   
      bool last = false;
      std::vector<BUFRhandle> bhl;

      int n = 100;
      for (int i = 0; i < n; i++)
        {
          bhl.emplace_back ();
          auto & bh = bhl.back ();
          bh.readFromFH (fh);
          if (! bh.isValid ())
            last = true;
        }

#pragma omp parallel for
      for (size_t i = 0; i < bhl.size (); i++)
        bhl[i].unpack ();

      if (0)
      for (auto & bh : bhl)
        {
          bh.get (lat_n, lat);
          bh.get (lon_n, lon);
          bh.get (val_n, val);
          bh.clear ();
        }

      bhl.clear ();

      printf ("%d\n", mess++);

      if (last)
        break;
    }   

  printf ("end\n");

  exit (0);
}

};


void GeoPoints::setup (const OptionsGeoPoints & o)
{
  if (! o.on)
    return;

  opts = o;

//netCDF::NcFile nc (opts.path, netCDF::NcFile::read);

  std::string path = opts.path;
  FILE * fh = fopen (path.c_str (), "r");

  if ((opts.lon == "") || (opts.lat == "") || (opts.val == ""))
    throw std::runtime_error (std::string ("Missing field value"));


//std::vector<float> lon = readNC (nc, opts.lon), 
//                   lat = readNC (nc, opts.lat),
//                   val = readNC (nc, opts.val);

  std::vector<float> lon, lat, val;

  readBUFR (fh, opts.lon, opts.lat, opts.val, lon, lat, val);

  fclose (fh);

  if ((lon.size () != lat.size ()) || (lon.size () != val.size ()))
    throw std::runtime_error (std::string ("Size mismatch"));

  Points::setup (opts.points, lon, lat, val);
}

void GeoPoints::render (const View & view, const OptionsLight & light) const
{
  if (opts.time.on)
    {
      Points::render (view, light, 0, opts.time.rate * time);
    }
  else
    {
      Points::render (view, light);
    }
}

void GeoPoints::update ()
{
  time = time + 1;
}

}
