#include "glGribGeoPoints.h"
#include "glGribTrigonometry.h"
#include "glGribResolve.h"
#include "glGribSQLite.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <stdlib.h>

#include <netcdf>

namespace
{

std::vector<float> readNC (const netCDF::NcFile & nc, const std::string & name)
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

};


void glGrib::GeoPoints::setup (const glGrib::OptionsGeoPoints & o)
{
  if (! o.on)
    return;

  opts = o;

  netCDF::NcFile nc (opts.path, netCDF::NcFile::read);

  if ((opts.lon == "") || (opts.lat == "") || (opts.val == ""))
    throw std::runtime_error (std::string ("Missing field value"));


  std::vector<float> lon = readNC (nc, opts.lon), 
                     lat = readNC (nc, opts.lat),
                     val = readNC (nc, opts.val);

  if ((lon.size () != lat.size ()) || (lon.size () != val.size ()))
    throw std::runtime_error (std::string ("Size mismatch"));

  glGrib::Points::setup (opts.points, lon, lat, val);
}

void glGrib::GeoPoints::render (const glGrib::View & view, const glGrib::OptionsLight & light) const
{
  if (opts.time.on)
    {
      glGrib::Points::render (view, light, 0, opts.time.rate * time);
    }
  else
    {
      glGrib::Points::render (view, light);
    }
}

void glGrib::GeoPoints::update ()
{
  time = time + 1;
}
